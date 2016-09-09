#include "service-skeleton.hh"

#include <stdexcept>

#include <core/dbus/message.h>
#include <core/dbus/object.h>
#include <core/dbus/types/object_path.h>
#include <core/dbus/types/variant.h>
#include <sys/apparmor.h>

#include <mediascanner/Album.hh>
#include <mediascanner/Filter.hh>
#include <mediascanner/MediaFile.hh>
#include <mediascanner/MediaStore.hh>

#include "dbus-interface.hh"
#include "dbus-codec.hh"

using core::dbus::Message;
using core::dbus::types::ObjectPath;
using core::dbus::types::Variant;

namespace mediascanner {
namespace dbus {

struct BusDaemon {
    static const std::string &name() {
        static std::string s = "org.freedesktop.DBus";
        return s;
    }

    struct GetConnectionCredentials {
        typedef BusDaemon Interface;

        static const std::string &name() {
            static std::string s = "GetConnectionCredentials";
            return s;
        }

        static const std::chrono::milliseconds default_timeout() {
            return std::chrono::seconds{1};
        }
    };
};

struct ServiceSkeleton::Private {
    ServiceSkeleton *impl;
    std::shared_ptr<MediaStore> store;
    core::dbus::Object::Ptr object;

    Private(ServiceSkeleton *impl, std::shared_ptr<MediaStore> store) :
        impl(impl),
        store(store),
        object(impl->access_service()->add_object_for_path(
                   core::dbus::traits::Service<MediaStoreService>::object_path())) {
        object->install_method_handler<MediaStoreInterface::Lookup>(
            std::bind(
                &Private::handle_lookup,
                this,
                std::placeholders::_1));
        object->install_method_handler<MediaStoreInterface::Query>(
            std::bind(
                &Private::handle_query,
                this,
                std::placeholders::_1));
        object->install_method_handler<MediaStoreInterface::QueryAlbums>(
            std::bind(
                &Private::handle_query_albums,
                this,
                std::placeholders::_1));
        object->install_method_handler<MediaStoreInterface::QueryArtists>(
            std::bind(
                &Private::handle_query_artists,
                this,
                std::placeholders::_1));
        object->install_method_handler<MediaStoreInterface::GetAlbumSongs>(
            std::bind(
                &Private::handle_get_album_songs,
                this,
                std::placeholders::_1));
        object->install_method_handler<MediaStoreInterface::GetETag>(
            std::bind(
                &Private::handle_get_etag,
                this,
                std::placeholders::_1));
        object->install_method_handler<MediaStoreInterface::ListSongs>(
            std::bind(
                &Private::handle_list_songs,
                this,
                std::placeholders::_1));
        object->install_method_handler<MediaStoreInterface::ListAlbums>(
            std::bind(
                &Private::handle_list_albums,
                this,
                std::placeholders::_1));
        object->install_method_handler<MediaStoreInterface::ListArtists>(
            std::bind(
                &Private::handle_list_artists,
                this,
                std::placeholders::_1));
        object->install_method_handler<MediaStoreInterface::ListAlbumArtists>(
            std::bind(
                &Private::handle_list_album_artists,
                this,
                std::placeholders::_1));
        object->install_method_handler<MediaStoreInterface::ListGenres>(
            std::bind(
                &Private::handle_list_genres,
                this,
                std::placeholders::_1));
        object->install_method_handler<MediaStoreInterface::HasMedia>(
            std::bind(
                &Private::handle_has_media,
                this,
                std::placeholders::_1));
    }

    std::string get_client_apparmor_context(const Message::Ptr &message) {
        if (!aa_is_enabled()) {
            return "unconfined";
        }
        auto service = core::dbus::Service::use_service(
            impl->access_bus(), "org.freedesktop.DBus");
        auto obj = service->object_for_path(
            ObjectPath("/org/freedesktop/DBus"));

        core::dbus::Result<std::map<std::string,Variant>> result;
        try {
            result = obj->invoke_method_synchronously<BusDaemon::GetConnectionCredentials,std::map<std::string,Variant>>(message->sender());
        } catch (const std::exception &e) {
            fprintf(stderr, "Error getting connection credentials: %s\n", e.what());
            return std::string();
        }
        if (result.is_error()) {
            fprintf(stderr, "Error getting connection credentials: %s\n", result.error().print().c_str());
            return std::string();
        }
        const auto& creds = result.value();
        auto it = creds.find("LinuxSecurityLabel");
        if (it == creds.end()) {
            fprintf(stderr, "Connection credentials don't include security label");
            return std::string();
        }
        std::vector<int8_t> label;
        try {
            label = it->second.as<std::vector<int8_t>>();
        } catch (const std::exception &e) {
            fprintf(stderr, "Could not convert security label to byte array");
            return std::string();
        }
        return std::string(aa_splitcon(reinterpret_cast<char*>(&label[0]), nullptr));
    }

    bool does_client_have_access(const std::string &context, MediaType type) {
        if (context.empty()) {
            // Deny access if we don't have a context
            return false;
        }
        if (context == "unconfined") {
            // Unconfined
            return true;
        }

        auto pos = context.find_first_of('_');
        if (pos == std::string::npos) {
            fprintf(stderr, "Badly formed AppArmor context: %s\n", context.c_str());
            return false;
        }
        const std::string pkgname = context.substr(0, pos);

        // TODO: when the trust store lands, check it to see if this
        // app can access the index.
        if (type == AudioMedia && pkgname == "com.ubuntu.music") {
            return true;
        }
        return false;
    }

    bool check_access(const Message::Ptr &message, MediaType type) {
        const std::string context = get_client_apparmor_context(message);
        bool have_access = does_client_have_access(context, type);
        if (!have_access) {
            auto reply = Message::make_error(
                message, MediaStoreInterface::Errors::Unauthorized::name(), "Unauthorized");
            impl->access_bus()->send(reply);
        }
        return have_access;
    }

    void handle_lookup(const Message::Ptr &message) {
        if (!check_access(message, AllMedia))
            return;

        std::string filename;
        message->reader() >> filename;
        Message::Ptr reply;
        try {
            MediaFile file = store->lookup(filename);
            reply = Message::make_method_return(message);
            reply->writer() << file;
        } catch (const std::exception &e) {
            reply = Message::make_error(
                message, MediaStoreInterface::Errors::Error::name(),
                e.what());
        }
        impl->access_bus()->send(reply);
    }

    void handle_query(const Message::Ptr &message) {
        std::string query;
        int32_t type;
        Filter filter;
        message->reader() >> query >> type >> filter;

        if (!check_access(message, (MediaType)type))
            return;

        Message::Ptr reply;
        try {
            auto results = store->query(query, (MediaType)type, filter);
            reply = Message::make_method_return(message);
            reply->writer() << results;
        } catch (const std::exception &e) {
            reply = Message::make_error(
                message, MediaStoreInterface::Errors::Error::name(),
                e.what());
        }
        impl->access_bus()->send(reply);
    }

    void handle_query_albums(const Message::Ptr &message) {
        if (!check_access(message, AudioMedia))
            return;

        std::string query;
        Filter filter;
        message->reader() >> query >> filter;
        Message::Ptr reply;
        try {
            auto albums = store->queryAlbums(query, filter);
            reply = Message::make_method_return(message);
            reply->writer() << albums;
        } catch (const std::exception &e) {
            reply = Message::make_error(
                message, MediaStoreInterface::Errors::Error::name(),
                e.what());
        }
        impl->access_bus()->send(reply);
    }

    void handle_query_artists(const Message::Ptr &message) {
        if (!check_access(message, AudioMedia))
            return;

        std::string query;
        Filter filter;
        message->reader() >> query >> filter;
        Message::Ptr reply;
        try {
            auto artists = store->queryArtists(query, filter);
            reply = Message::make_method_return(message);
            reply->writer() << artists;
        } catch (const std::exception &e) {
            reply = Message::make_error(
                message, MediaStoreInterface::Errors::Error::name(),
                e.what());
        }
        impl->access_bus()->send(reply);
    }

    void handle_get_album_songs(const Message::Ptr &message) {
        if (!check_access(message, AudioMedia))
            return;

        Album album("", "");
        message->reader() >> album;
        Message::Ptr reply;
        try {
            auto results = store->getAlbumSongs(album);
            reply = Message::make_method_return(message);
            reply->writer() << results;
        } catch (const std::exception &e) {
            reply = Message::make_error(
                message, MediaStoreInterface::Errors::Error::name(),
                e.what());
        }
        impl->access_bus()->send(reply);
    }

    void handle_get_etag(const Message::Ptr &message) {
        if (!check_access(message, AllMedia))
            return;

        std::string filename;
        message->reader() >> filename;

        Message::Ptr reply;
        try {
            std::string etag = store->getETag(filename);
            reply = Message::make_method_return(message);
            reply->writer() << etag;
        } catch (const std::exception &e) {
            reply = Message::make_error(
                message, MediaStoreInterface::Errors::Error::name(),
                e.what());
        }
        impl->access_bus()->send(reply);
    }

    void handle_list_songs(const Message::Ptr &message) {
        if (!check_access(message, AudioMedia))
            return;

        Filter filter;
        message->reader() >> filter;
        Message::Ptr reply;
        try {
            auto results = store->listSongs(filter);
            reply = Message::make_method_return(message);
            reply->writer() << results;
        } catch (const std::exception &e) {
            reply = Message::make_error(
                message, MediaStoreInterface::Errors::Error::name(),
                e.what());
        }
        impl->access_bus()->send(reply);
    }

    void handle_list_albums(const Message::Ptr &message) {
        if (!check_access(message, AudioMedia))
            return;

        Filter filter;
        message->reader() >> filter;
        Message::Ptr reply;
        try {
            auto albums = store->listAlbums(filter);
            reply = Message::make_method_return(message);
            reply->writer() << albums;
        } catch (const std::exception &e) {
            reply = Message::make_error(
                message, MediaStoreInterface::Errors::Error::name(),
                e.what());
        }
        impl->access_bus()->send(reply);
    }

    void handle_list_artists(const Message::Ptr &message) {
        if (!check_access(message, AudioMedia))
            return;

        Filter filter;
        message->reader() >> filter;
        Message::Ptr reply;
        try {
            auto artists = store->listArtists(filter);
            reply = Message::make_method_return(message);
            reply->writer() << artists;
        } catch (const std::exception &e) {
            reply = Message::make_error(
                message, MediaStoreInterface::Errors::Error::name(),
                e.what());
        }
        impl->access_bus()->send(reply);
    }

    void handle_list_album_artists(const Message::Ptr &message) {
        if (!check_access(message, AudioMedia))
            return;

        Filter filter;
        message->reader() >> filter;
        Message::Ptr reply;
        try {
            auto artists = store->listAlbumArtists(filter);
            reply = Message::make_method_return(message);
            reply->writer() << artists;
        } catch (const std::exception &e) {
            reply = Message::make_error(
                message, MediaStoreInterface::Errors::Error::name(),
                e.what());
        }
        impl->access_bus()->send(reply);
    }

    void handle_list_genres(const Message::Ptr &message) {
        if (!check_access(message, AudioMedia))
            return;

        Filter filter;
        message->reader() >> filter;
        Message::Ptr reply;
        try {
            auto genres = store->listGenres(filter);
            reply = Message::make_method_return(message);
            reply->writer() << genres;
        } catch (const std::exception &e) {
            reply = Message::make_error(
                message, MediaStoreInterface::Errors::Error::name(),
                e.what());
        }
        impl->access_bus()->send(reply);
    }

    void handle_has_media(const Message::Ptr &message) {
        int32_t type;
        message->reader() >> type;

        if (!check_access(message, static_cast<MediaType>(type)))
            return;
        Message::Ptr reply;
        try {
            bool result = store->hasMedia(static_cast<MediaType>(type));
            reply = Message::make_method_return(message);
            reply->writer() << result;
        } catch (const std::exception &e) {
            reply = Message::make_error(
                message, MediaStoreInterface::Errors::Error::name(),
                e.what());
        }
        impl->access_bus()->send(reply);
    }
};

ServiceSkeleton::ServiceSkeleton(core::dbus::Bus::Ptr bus,
                                 std::shared_ptr<MediaStore> store) :
    core::dbus::Skeleton<MediaStoreService>(bus),
    p(new Private(this, store)) {
}

ServiceSkeleton::~ServiceSkeleton() {
}

void ServiceSkeleton::run() {
    access_bus()->run();
}

void ServiceSkeleton::stop() {
    access_bus()->stop();
}

}
}

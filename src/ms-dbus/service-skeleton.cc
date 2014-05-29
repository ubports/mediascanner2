#include <stdexcept>

#include <core/dbus/message.h>
#include <core/dbus/object.h>
#include <core/dbus/types/object_path.h>
#include <sys/apparmor.h>

#include <mediascanner/Album.hh>
#include <mediascanner/MediaFile.hh>
#include <mediascanner/MediaStore.hh>

#include "dbus-interface.hh"
#include "dbus-codec.hh"
#include "service-skeleton.hh"

using core::dbus::Message;

namespace mediascanner {
namespace dbus {

struct Apparmor {
    static const std::string &name() {
        static std::string s = "org.freedesktop.DBus";
        return s;
    }

    struct GetConnectionAppArmorSecurityContext {
        typedef Apparmor Interface;

        static const std::string &name() {
            static std::string s = "GetConnectionAppArmorSecurityContext";
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
    }

    std::string get_client_apparmor_context(const Message::Ptr &message) {
        if (!aa_is_enabled()) {
            return "unconfined";
        }
        auto service = core::dbus::Service::use_service(
            impl->access_bus(), "org.freedesktop.DBus");
        auto obj = service->object_for_path(
            core::dbus::types::ObjectPath("/org/freedesktop/DBus"));

        core::dbus::Result<std::string> result;
        try {
            result = obj->invoke_method_synchronously<Apparmor::GetConnectionAppArmorSecurityContext, std::string>(message->sender());
        } catch (const std::runtime_error &e) {
            fprintf(stderr, "Error getting apparmor context: %s\n", e.what());
            return std::string();
        }
        if (result.is_error()) {
            fprintf(stderr, "Error getting apparmor context: %s\n", result.error().print().c_str());
            return std::string();
        }
        return result.value();
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
        int32_t limit;
        message->reader() >> query >> type >> limit;

        if (!check_access(message, (MediaType)type))
            return;

        Message::Ptr reply;
        try {
            auto results = store->query(query, (MediaType)type, limit);
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
        int32_t limit;
        message->reader() >> query >> limit;
        Message::Ptr reply;
        try {
            auto albums = store->queryAlbums(query, limit);
            reply = Message::make_method_return(message);
            reply->writer() << albums;
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

        std::string artist, album, album_artist;
        int32_t limit;
        message->reader() >> artist >> album >> album_artist >> limit;
        Message::Ptr reply;
        try {
            auto results = store->listSongs(artist, album, album_artist, limit);
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

        std::string artist, album_artist;
        int32_t limit;
        message->reader() >> artist >> album_artist >> limit;
        Message::Ptr reply;
        try {
            auto albums = store->listAlbums(artist, album_artist, limit);
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

        bool album_artists;
        int32_t limit;
        message->reader() >> album_artists >> limit;
        Message::Ptr reply;
        try {
            auto artists = store->listArtists(album_artists, limit);
            reply = Message::make_method_return(message);
            reply->writer() << artists;
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

#include <stdexcept>

#include <core/dbus/message.h>
#include <core/dbus/object.h>
#include <core/dbus/types/object_path.h>

#include <mediascanner/Album.hh>
#include <mediascanner/Filter.hh>
#include <mediascanner/MediaFile.hh>
#include <mediascanner/MediaStore.hh>

#include "dbus-interface.hh"
#include "dbus-codec.hh"
#include "service-skeleton.hh"

using core::dbus::Message;

namespace mediascanner {
namespace dbus {

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

    void handle_lookup(const Message::Ptr &message) {
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
        std::string artist, album, album_artist;
        int32_t limit;

        message->reader() >> artist >> album >> album_artist >> limit;
        Message::Ptr reply;
        try {
            Filter filter;
            filter.setArtist(artist);
            filter.setAlbum(album);
            filter.setAlbumArtist(album_artist);
            auto results = store->listSongs(filter, limit);
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
        std::string artist, album_artist;
        int32_t limit;

        message->reader() >> artist >> album_artist >> limit;
        Message::Ptr reply;
        try {
            Filter filter;
            filter.setArtist(artist);
            filter.setAlbumArtist(album_artist);
            auto albums = store->listAlbums(filter, limit);
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
        bool album_artists;
        int32_t limit;

        message->reader() >> album_artists >> limit;
        Message::Ptr reply;
        try {
            Filter filter;
            std::vector<std::string>artists;
            if (album_artists)
                artists = store->listAlbumArtists(filter, limit);
            else
                artists = store->listArtists(filter, limit);
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

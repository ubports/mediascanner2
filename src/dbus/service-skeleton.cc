#include <stdexcept>

#include <core/dbus/message.h>
#include <core/dbus/object.h>
#include <core/dbus/types/object_path.h>

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
                   core::dbus::traits::Service<ScannerService>::object_path())) {
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
    }

    void handle_lookup(const Message::Ptr &message) {
        std::string filename;
        message->reader() >> filename;

        Message::Ptr reply;
        try {
            MediaFile file = store->lookup(filename);
            reply = Message::make_method_return(message);
            reply->writer() << file;
        } catch (std::runtime_error &e) {
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
        } catch (std::runtime_error &e) {
            reply = Message::make_error(
                message, MediaStoreInterface::Errors::Error::name(),
                e.what());
        }
        impl->access_bus()->send(reply);
    }
};

ServiceSkeleton::ServiceSkeleton(core::dbus::Bus::Ptr bus,
                                 std::shared_ptr<MediaStore> store) :
    core::dbus::Skeleton<ScannerService>(bus),
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

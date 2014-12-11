#ifndef EXTRACTOR_DBUS_MARSHAL_HH
#define EXTRACTOR_DBUS_MARSHAL_HH

#include <glib.h>

namespace mediascanner {

class MediaFile;

GVariant *media_to_variant(const MediaFile &media);
MediaFile media_from_variant(GVariant *variant);

}

#endif

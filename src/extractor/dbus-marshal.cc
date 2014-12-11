#include "dbus-marshal.hh"
#include <mediascanner/MediaFile.hh>
#include <mediascanner/MediaFileBuilder.hh>

#include <stdexcept>

namespace mediascanner {

GVariant *media_to_variant(const MediaFile &media) {
    GVariantBuilder builder;

    g_variant_builder_init(&builder, G_VARIANT_TYPE("(sssssssssiiiiiddi)"));
    g_variant_builder_add(&builder, "s", media.getFileName().c_str());
    g_variant_builder_add(&builder, "s", media.getContentType().c_str());
    g_variant_builder_add(&builder, "s", media.getETag().c_str());
    g_variant_builder_add(&builder, "s", media.getTitle().c_str());
    g_variant_builder_add(&builder, "s", media.getAuthor().c_str());
    g_variant_builder_add(&builder, "s", media.getAlbum().c_str());
    g_variant_builder_add(&builder, "s", media.getAlbumArtist().c_str());
    g_variant_builder_add(&builder, "s", media.getDate().c_str());
    g_variant_builder_add(&builder, "s", media.getGenre().c_str());
    g_variant_builder_add(&builder, "i", media.getDiscNumber());
    g_variant_builder_add(&builder, "i", media.getTrackNumber());
    g_variant_builder_add(&builder, "i", media.getDuration());
    g_variant_builder_add(&builder, "i", media.getWidth());
    g_variant_builder_add(&builder, "i", media.getHeight());
    g_variant_builder_add(&builder, "d", media.getLatitude());
    g_variant_builder_add(&builder, "d", media.getLongitude());
    g_variant_builder_add(&builder, "i", static_cast<int>(media.getType()));

    return g_variant_builder_end(&builder);
}

MediaFile media_from_variant(GVariant *variant) {
    if (!g_variant_is_of_type(variant, G_VARIANT_TYPE("(sssssssssiiiiiddi)"))) {
        throw std::runtime_error("variant is of wrong type");
    }

    const char *filename = nullptr, *content_type = nullptr, *etag = nullptr,
        *title = nullptr, *author = nullptr, *album = nullptr,
        *album_artist = nullptr, *date = nullptr, *genre = nullptr;
    gint32 disc_number = 0, track_number = 0, duration = 0,
        width = 0, height = 0, type = 0;
    double latitude = 0, longitude = 0;
    g_variant_get(variant, "(&s&s&s&s&s&s&s&s&siiiiiddi)",
                  &filename, &content_type, &etag, &title, &author,
                  &album, &album_artist, &date, &genre, &disc_number,
                  &track_number, &duration, &width, &height,
                  &latitude, &longitude, &type, nullptr);

    return MediaFileBuilder(filename)
        .setContentType(content_type)
        .setETag(etag)
        .setTitle(title)
        .setAuthor(author)
        .setAlbum(album)
        .setAlbumArtist(album_artist)
        .setDate(date)
        .setGenre(genre)
        .setDiscNumber(disc_number)
        .setTrackNumber(track_number)
        .setDuration(duration)
        .setWidth(width)
        .setHeight(height)
        .setLatitude(latitude)
        .setLongitude(longitude)
        .setType(static_cast<MediaType>(type));
}

}

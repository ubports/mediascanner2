/*
 * Copyright (C) 2013 Canonical, Ltd.
 *
 * Authors:
 *    James Henstridge <james.henstridge@canonical.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef MEDIASCANNER_DBUS_CODEC_HH
#define MEDIASCANNER_DBUS_CODEC_HH

#include <string>

#include <core/dbus/codec.h>
#include <core/dbus/helper/type_mapper.h>

namespace mediascanner {
class MediaFile;
class Album;
class Filter;
}

namespace core {
namespace dbus {

template <>
struct Codec<mediascanner::MediaFile> {
    static void encode_argument(Message::Writer &out, const mediascanner::MediaFile &file);
    static void decode_argument(Message::Reader &in, mediascanner::MediaFile &file);
};

template <>
struct Codec<mediascanner::Album> {
    static void encode_argument(Message::Writer &out, const mediascanner::Album &album);
    static void decode_argument(Message::Reader &in, mediascanner::Album &album);
};

template <>
struct Codec<mediascanner::Filter> {
    static void encode_argument(Message::Writer &out, const mediascanner::Filter &filter);
    static void decode_argument(Message::Reader &in, mediascanner::Filter &filter);
};

namespace helper {

template<>
struct TypeMapper<mediascanner::MediaFile> {
    constexpr static ArgumentType type_value() {
        return ArgumentType::structure;
    }
    constexpr static bool is_basic_type() {
        return false;
    }
    constexpr static bool requires_signature() {
        return true;
    }
    static const std::string &signature() {
        static const std::string s = "(sssssssssiiiiiddbti)";
        return s;
    }
};

template<>
struct TypeMapper<mediascanner::Album> {
    constexpr static ArgumentType type_value() {
        return ArgumentType::structure;
    }
    constexpr static bool is_basic_type() {
        return false;
    }
    constexpr static bool requires_signature() {
        return true;
    }
    static const std::string &signature() {
        static const std::string s = "(sssssbi)";
        return s;
    }
};

template<>
struct TypeMapper<mediascanner::Filter> {
    constexpr static ArgumentType type_value() {
        return ArgumentType::array;
    }
    constexpr static bool is_basic_type() {
        return false;
    }
    constexpr static bool requires_signature() {
        return true;
    }
    static const std::string &signature() {
        static const std::string s = "a{sv}";
        return s;
    }
};

}

}
}
#endif

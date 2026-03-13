#pragma once

#include <magic_enum.hpp>
#include <memory>
#include <nlohmann/json.hpp>

namespace nlohmann {
    // template <class T>
    // void from_json(const nlohmann::json& j, std::shared_ptr<T>& v) {
    //     if (j.is_null())
    //         v = nullptr;
    //     else
    //         v = std::make_shared<T>(j.get<T>());
    // }

    /**
     * @brief nlohmann ADL serialiser for std::unique_ptr<T>.
     *
     * A null JSON value deserialises to @c nullptr; any other value is
     * deserialised as @c T and wrapped in a unique_ptr.
     *
     * @tparam T Pointee type.
     */
    template <typename T>
    struct adl_serializer<std::unique_ptr<T>> {
        /// @brief Deserialises @p j into a unique_ptr, or sets it to @c nullptr.
        static void from_json(json const& j, std::unique_ptr<T>& ptr) {
            if (j.is_null()) {
                ptr = nullptr;
            } else {
                ptr = std::make_unique<T>(j.get<T>());
            }
        }
        /// @brief Serialises a unique_ptr: writes @c null if empty, otherwise the pointee.
        static void to_json(json& j, const std::unique_ptr<T>& ptr) {
            if (ptr.get()) {
                j = *ptr;
            } else {
                j = nullptr;
            }
        }
    };

    /**
     * @brief nlohmann ADL serialiser for std::shared_ptr<T>.
     *
     * A null JSON value deserialises to @c nullptr; any other value is
     * deserialised as @c T and wrapped in a shared_ptr.
     *
     * @tparam T Pointee type.
     */
    template <typename T>
    struct adl_serializer<std::shared_ptr<T>> {
        /// @brief Deserialises @p j into a shared_ptr, or sets it to @c nullptr.
        static void from_json(json const& j, std::shared_ptr<T>& ptr) {
            if (j.is_null()) {
                ptr = nullptr;
            } else {
                ptr = std::make_shared<T>(j.get<T>());
            }
        }
        /// @brief Serialises a shared_ptr: writes @c null if empty, otherwise the pointee.
        static void to_json(json& j, const std::shared_ptr<T>& ptr) {
            if (ptr.get()) {
                j = *ptr;
            } else {
                j = nullptr;
            }
        }
    };
    // template <typename T>
    // struct adl_serializer<std::shared_ptr<const T>> {
    //     static void from_json(json& j, std::shared_ptr<const T>& ptr) {
    //         if (j.is_null()) {
    //             ptr == nullptr;
    //         } else {
    //             ptr = std::make_shared<const T>(j.get<T>());
    //         }
    //     }
    //     static void to_json(json& j, const std::shared_ptr<const T>& ptr) {
    //         if (ptr.get()) {
    //             j = *ptr;
    //         } else {
    //             j = nullptr;
    //         }
    //     }
    // };
    // template <typename T>
    // struct adl_serializer<std::optional<T>> {
    //     static void to_json(json& j, const std::optional<T>& opt) {
    //         if (opt.has_value()) {
    //             j = nullptr;
    //         } else {
    //             j = *opt;
    //         }
    //     }

    //    static void from_json(const json& j, std::optional<T>& opt) {
    //        if (j.is_null()) {
    //            opt = std::nullopt;
    //        } else {
    //            opt = j.get<T>();
    //        }
    //    }
    //};

    /**
     * @brief nlohmann ADL serialiser for enum types via magic_enum.
     *
     * Enum values are serialised as their string name; deserialisation
     * converts the string back using @c magic_enum::enum_cast.
     *
     * @tparam T Enum type.
     */
    template <typename T>
    struct adl_serializer<T, typename std::enable_if<std::is_enum<T>::value>::type> {
        /// @brief Serialises @p e as its string representation.
        static void to_json(json& j, const T& e) {
            std::string v = std::string(magic_enum::enum_name(e));
            j = v;
        }

        /// @brief Deserialises a string into an enum value.
        /// @throws nlohmann::json::other_error if @p j contains an unrecognised enum string.
        static void from_json(const json& j, T& e) {
            std::string v;
            j.get_to(v);
            auto enumValue = magic_enum::enum_cast<T>(v);
            if (enumValue.has_value()) {
                e = enumValue.value();
            } else {
                throw nlohmann::json::other_error::create(
                    // NOLINTNEXTLINE
                    501,
                    "invalid enum value '" + v + "' for type " + std::string(magic_enum::enum_type_name<T>()),
                    &j);
            }
        }
    };
}

// template <typename T, typename = std::enable_if_t<std::is_enum<T>::value>>
// inline void to_json(nlohmann::json& j, T const& e) {
//     std::string v = std::string(magic_enum::enum_name(e));
//     j = v;
// }
//
// template <typename T, typename = std::enable_if_t<std::is_enum<T>::value>>
// inline void from_json(nlohmann::json const& j, T& e) {
//     std::string v;
//     j.get_to(v);
//     e = magic_enum::enum_cast<T>(v).value();
// }

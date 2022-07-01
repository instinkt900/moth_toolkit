#pragma once

namespace nlohmann {
    //template <class T>
    //void from_json(const nlohmann::json& j, std::shared_ptr<T>& v) {
    //    if (j.is_null())
    //        v = nullptr;
    //    else
    //        v = std::make_shared<T>(j.get<T>());
    //}

    template <typename T>
    struct adl_serializer<std::unique_ptr<T>> {
        static void from_json(json const& j, std::unique_ptr<T>& ptr) {
            if (j.is_null()) {
                ptr = nullptr;
            } else {
                ptr = std::make_unique<T>(j.get<T>());
            }
        }
        static void to_json(json& j, const std::unique_ptr<T>& ptr) {
            if (ptr.get()) {
                j = *ptr;
            } else {
                j = nullptr;
            }
        }
    };
    template <typename T>
    struct adl_serializer<std::shared_ptr<T>> {
        static void from_json(json const & j, std::shared_ptr<T>& ptr) {
            if (j.is_null()) {
                ptr = nullptr;
            } else {
                ptr = std::make_shared<T>(j.get<T>());
            }
        }
        static void to_json(json& j, const std::shared_ptr<T>& ptr) {
            if (ptr.get()) {
                j = *ptr;
            } else {
                j = nullptr;
            }
        }
    };
    //template <typename T>
    //struct adl_serializer<std::shared_ptr<const T>> {
    //    static void from_json(json& j, std::shared_ptr<const T>& ptr) {
    //        if (j.is_null()) {
    //            ptr == nullptr;
    //        } else {
    //            ptr = std::make_shared<const T>(j.get<T>());
    //        }
    //    }
    //    static void to_json(json& j, const std::shared_ptr<const T>& ptr) {
    //        if (ptr.get()) {
    //            j = *ptr;
    //        } else {
    //            j = nullptr;
    //        }
    //    }
    //};
    //template <typename T>
    //struct adl_serializer<std::optional<T>> {
    //    static void to_json(json& j, const std::optional<T>& opt) {
    //        if (opt.has_value()) {
    //            j = nullptr;
    //        } else {
    //            j = *opt;
    //        }
    //    }

    //    static void from_json(const json& j, std::optional<T>& opt) {
    //        if (j.is_null()) {
    //            opt = std::nullopt;
    //        } else {
    //            opt = j.get<T>();
    //        }
    //    }
    //};

    template <typename T>
    struct adl_serializer<T, typename std::enable_if<std::is_enum<T>::value>::type> {
        static void to_json(json& j, const T& e) {
            std::string v = std::string(magic_enum::enum_name(e));
            j = v;
        }

        static void from_json(const json& j, T& e) {
            std::string v;
            j.get_to(v);
            auto enumValue = magic_enum::enum_cast<T>(v);
            if (enumValue.has_value()) {
                e = magic_enum::enum_cast<T>(v).value();
            }
        }
    };
}

//template <typename T, typename = std::enable_if_t<std::is_enum<T>::value>>
//inline void to_json(nlohmann::json& j, T const& e) {
//    std::string v = std::string(magic_enum::enum_name(e));
//    j = v;
//}
//
//template <typename T, typename = std::enable_if_t<std::is_enum<T>::value>>
//inline void from_json(nlohmann::json const& j, T& e) {
//    std::string v;
//    j.get_to(v);
//    e = magic_enum::enum_cast<T>(v).value();
//}

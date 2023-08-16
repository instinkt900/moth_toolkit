#pragma once

#include "ifont.h"

namespace moth_ui {
    class IFontFactory {
    public:
        virtual ~IFontFactory() = default;

        virtual std::shared_ptr<IFont> GetDefaultFont(int size) = 0;
        virtual std::vector<std::string> GetFontNameList() = 0;
        virtual std::shared_ptr<IFont> GetFont(char const* name, int size) = 0;
    };
}

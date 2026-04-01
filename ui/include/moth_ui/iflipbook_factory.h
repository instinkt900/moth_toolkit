#pragma once

#include "moth_ui/graphics/iflipbook.h"
#include <filesystem>
#include <memory>

namespace moth_ui {
    /**
     * @brief Abstract factory interface for loading flipbooks from descriptor files.
     *
     * Consumers of the UI library must provide a concrete implementation that
     * parses .flipbook.json descriptor files and loads the referenced sprite sheet.
     */
    class IFlipbookFactory {
    public:
        IFlipbookFactory() = default;
        IFlipbookFactory(IFlipbookFactory const&) = default;
        IFlipbookFactory(IFlipbookFactory&&) = default;
        IFlipbookFactory& operator=(IFlipbookFactory const&) = default;
        IFlipbookFactory& operator=(IFlipbookFactory&&) = default;
        virtual ~IFlipbookFactory() = default;

        /// @brief Discards all cached flipbooks, forcing subsequent loads from disk.
        virtual void FlushCache() = 0;

        /// @brief Loads a flipbook from a descriptor file.
        /// @param path Path to the .flipbook.json descriptor.
        /// @return Loaded flipbook, or nullptr on failure.
        virtual std::unique_ptr<IFlipbook> GetFlipbook(std::filesystem::path const& path) = 0;
    };
}

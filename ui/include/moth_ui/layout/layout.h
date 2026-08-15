#pragma once

#include "moth_ui/layout/layout_entity_group.h"
#include "moth_ui/moth_ui_fwd.h"

#include <utility>

namespace moth_ui {
    /**
     * @brief The root LayoutEntityGroup that represents a complete saved layout file.
     *
     * A Layout can be loaded from or saved to disk.  It extends
     * LayoutEntityGroup with a file path, a format version, and an arbitrary
     * JSON blob that editors and other tools may use for their own metadata.
     */
    class Layout : public LayoutEntityGroup {
    public:
        Layout();

        /// @brief Deep-copies the layout and its entity tree.
        std::shared_ptr<LayoutEntity> Clone(CloneType cloneType) override;

        /// @brief Returns @c LayoutEntityType::Layout.
        LayoutEntityType GetType() const override { return LayoutEntityType::Layout; }

        /// @brief Instantiates the layout's entity tree as a live node hierarchy.
        std::shared_ptr<Node> Instantiate(Context& context) override;

        nlohmann::json Serialize(SerializeContext const& context) const override;
        bool Deserialize(nlohmann::json const& json, SerializeContext const& context) override;

        /// @brief Returns the filesystem path from which this layout was loaded.
        std::filesystem::path const& GetLoadedPath() const { return m_loadedPath; }

        /// @brief Returns a mutable reference to the arbitrary editor metadata blob.
        nlohmann::json& GetExtraData() { return m_extraData; }

        /// @brief Result codes returned by Load().
        enum class LoadResult {
            Success,            ///< Layout loaded successfully.
            DoesNotExist,       ///< The specified file does not exist.
            IncorrectFormat,    ///< The file exists but could not be parsed.
            InstantiationFailed, ///< The layout was parsed but could not be instantiated into a node.
        };

        /// @brief Options controlling how a layout is read from disk.
        struct LoadOptions {
            bool binary = false; ///< Read as MessagePack binary; if false, parse as JSON.
        };

        /// @brief Options controlling how a layout is written to disk.
        struct SaveOptions {
            bool binary = false; ///< Write MessagePack binary (.mothb) instead of JSON.
            bool pretty = false; ///< Pretty-print JSON output (ignored when binary is true).
        };

        /**
         * @brief Loads a layout from a file as JSON (default format).
         * @param path Path to the layout file.
         * @return A pair of the loaded layout (or nullptr on failure) and the result code.
         */
        static std::pair<std::shared_ptr<Layout>, LoadResult> Load(std::filesystem::path const& path);

        /**
         * @brief Loads a layout from a file with explicit format options.
         * @param path    Path to the layout file.
         * @param options Specifies whether to read as binary (MessagePack) or text (JSON).
         * @return A pair of the loaded layout (or nullptr on failure) and the result code.
         */
        static std::pair<std::shared_ptr<Layout>, LoadResult> Load(std::filesystem::path const& path, LoadOptions const& options);

        /// @brief Saves this layout to a file using default SaveOptions (JSON, non-pretty).
        ///        Equivalent to calling Save(path, SaveOptions{}).
        bool Save(std::filesystem::path const& path) const;

        /**
         * @brief Saves this layout to a file.
         * @param path    Destination path.
         * @param options Controls the output format. SaveOptions defaults to JSON
         *                (binary = false) with compact output (pretty = false).
         * @return @c true on success.
         */
        bool Save(std::filesystem::path const& path, SaveOptions const& options) const;

        static int const Version;                     ///< Current file format version.
        static std::string const Extension;           ///< JSON file extension without the leading dot.
        static std::string const FullExtension;       ///< JSON full file extension including the leading dot.
        static std::string const BinaryExtension;     ///< Binary file extension without the leading dot.
        static std::string const FullBinaryExtension; ///< Binary full file extension including the leading dot.

        Layout(Layout const&) = default;
        Layout(Layout&&) = default;
        Layout& operator=(Layout const&) = delete;
        Layout& operator=(Layout&&) = default;
        ~Layout() override = default;

    private:
        std::filesystem::path m_loadedPath;
        nlohmann::json m_extraData; // extra metadata for editor and other tools
    };
}

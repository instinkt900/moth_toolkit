#pragma once

#include "moth_ui/layout/layout_entity_group.h"
#include "moth_ui/moth_ui_fwd.h"

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

        std::shared_ptr<LayoutEntity> Clone(CloneType cloneType) override;

        /// @brief Returns @c LayoutEntityType::Layout.
        LayoutEntityType GetType() const override { return LayoutEntityType::Layout; }

        std::unique_ptr<Node> Instantiate(Context& context) override;

        nlohmann::json Serialize(SerializeContext const& context) const override;
        bool Deserialize(nlohmann::json const& json, SerializeContext const& context) override;

        /// @brief Returns the filesystem path from which this layout was loaded.
        std::filesystem::path const& GetLoadedPath() const { return m_loadedPath; }

        /// @brief Returns a mutable reference to the arbitrary editor metadata blob.
        nlohmann::json& GetExtraData() { return m_extraData; }

        /// @brief Result codes returned by Load().
        enum class LoadResult {
            Success,         ///< Layout loaded successfully.
            NoOutput,        ///< @p outLayout was @c nullptr; load was still attempted.
            DoesNotExist,    ///< The specified file does not exist.
            IncorrectFormat, ///< The file exists but could not be parsed.
        };

        /// @brief Options controlling how a layout is written to disk.
        struct SaveOptions {
            bool binary = false; ///< Write MessagePack binary (.mothb) instead of JSON.
            bool pretty = false; ///< Pretty-print JSON output (ignored when binary is true).
        };

        /**
         * @brief Loads a layout from a file.
         *
         * The format is detected from the file extension: @c .mothb is read as
         * MessagePack binary; all other extensions are parsed as JSON.
         *
         * @param path      Path to the layout file.
         * @param outLayout If non-null, receives the loaded Layout on success.
         * @return A LoadResult indicating success or the reason for failure.
         */
        static LoadResult Load(std::filesystem::path const& path, std::shared_ptr<Layout>* outLayout = nullptr);

        /**
         * @brief Saves this layout to a file.
         * @param path    Destination path.
         * @param options Controls binary/text format and JSON pretty-printing.
         * @return @c true on success.
         */
        bool Save(std::filesystem::path const& path) const;
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

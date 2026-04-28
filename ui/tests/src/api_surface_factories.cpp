// Pins the factory interface and Context signatures.

#include "moth_ui/moth_ui.h"

#include <catch2/catch_all.hpp>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

using namespace moth_ui;

TEST_CASE("IImageFactory method signatures are stable", "[api][factories][iimage]") {
    std::unique_ptr<IImage> (IImageFactory::*getImg)(std::filesystem::path const&)
        = &IImageFactory::GetImage;
    (void)getImg;
    SUCCEED();
}

TEST_CASE("IFontFactory method signatures are stable", "[api][factories][ifont]") {
    void (IFontFactory::*addFont)(std::string const&, std::filesystem::path const&)
        = &IFontFactory::AddFont;
    void (IFontFactory::*removeFont)(std::string const&)
        = &IFontFactory::RemoveFont;
    void (IFontFactory::*loadProj)(std::filesystem::path const&)
        = &IFontFactory::LoadProject;
    void (IFontFactory::*saveProj)(std::filesystem::path const&)
        = &IFontFactory::SaveProject;
    std::filesystem::path (IFontFactory::*getCurPath)() const
        = &IFontFactory::GetCurrentProjectPath;
    void (IFontFactory::*clearFonts)()
        = &IFontFactory::ClearFonts;
    std::shared_ptr<IFont> (IFontFactory::*getDefault)(int)
        = &IFontFactory::GetDefaultFont;
    std::vector<std::string> (IFontFactory::*getList)() const
        = &IFontFactory::GetFontNameList;
    std::shared_ptr<IFont> (IFontFactory::*getFont)(std::string const&, int)
        = &IFontFactory::GetFont;
    std::filesystem::path (IFontFactory::*getFontPath)(std::string const&) const
        = &IFontFactory::GetFontPath;

    (void)addFont; (void)removeFont; (void)loadProj; (void)saveProj;
    (void)getCurPath; (void)clearFonts; (void)getDefault;
    (void)getList; (void)getFont; (void)getFontPath;
    SUCCEED();
}

TEST_CASE("IFlipbookFactory method signatures are stable", "[api][factories][iflipbook]") {
    std::unique_ptr<IFlipbook> (IFlipbookFactory::*get)(std::filesystem::path const&)
        = &IFlipbookFactory::GetFlipbook;
    (void)get;
    SUCCEED();
}

TEST_CASE("Context construction and getters are stable", "[api][context]") {
    // Three-argument form (fourth parameter defaults to nullptr) must also compile.
    static_assert(std::is_constructible_v<Context,
                                          IImageFactory*, IFontFactory*, IRenderer*>);
    // Four-argument form (explicit flipbook factory) must exist.
    static_assert(std::is_constructible_v<Context,
                                          IImageFactory*, IFontFactory*,
                                          IRenderer*, IFlipbookFactory*>);
    // Getter signatures
    IImageFactory&    (Context::*getImg)()  const = &Context::GetImageFactory;
    IFontFactory&     (Context::*getFnt)()  const = &Context::GetFontFactory;
    IRenderer&        (Context::*getRdr)()  const = &Context::GetRenderer;
    IFlipbookFactory* (Context::*getFb)()   const = &Context::GetFlipbookFactory;
    (void)getImg; (void)getFnt; (void)getRdr; (void)getFb;
    SUCCEED();
}

TEST_CASE("NodeFactory interface is stable", "[api][nodefactory]") {
    // Singleton accessor
    NodeFactory& (*get)() = &NodeFactory::Get;
    (void)get;
    // RegisterWidget
    std::string (NodeFactory::*reg)(std::string const&, NodeFactory::CreationFunction const&)
        = &NodeFactory::RegisterWidget;
    // Create overloads
    std::shared_ptr<Group> (NodeFactory::*createFromPath)(
        Context&, std::filesystem::path const&, int, int) = &NodeFactory::Create;
    std::shared_ptr<Group> (NodeFactory::*createFromGroup)(
        Context&, std::shared_ptr<LayoutEntityGroup>)     = &NodeFactory::Create;
    std::shared_ptr<Node>  (NodeFactory::*createFromEntity)(
        Context&, std::shared_ptr<LayoutEntity>)          = &NodeFactory::Create;
    (void)reg; (void)createFromPath; (void)createFromGroup; (void)createFromEntity;
    SUCCEED();
}

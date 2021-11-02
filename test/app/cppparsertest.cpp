/*
The MIT License (MIT)

Copyright (c) 2014

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "cppparser.h"
#include "compare.h"
#include "cppwriter.h"
#include "options.h"

#include <fstream>
#include <iostream>
#include <utility>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/system/config.hpp>

namespace bfs = boost::filesystem;
namespace bpo = boost::program_options;

//////////////////////////////////////////////////////////////////////////

static bool parseAndEmitFormatted(CppParser&       parser,
                                  const bfs::path& inputFilePath,
                                  const bfs::path& outputFilePath,
                                  const CppWriter& cppWriter)
{
  auto progUnit = parser.parseFile(inputFilePath.string().c_str());
  if (!progUnit)
    return false;
  bfs::create_directories(outputFilePath.parent_path());
  std::ofstream stm(outputFilePath.string());
  cppWriter.emit(progUnit.get(), stm);

  return true;
}

static bool performParsing(CppParser& parser, const std::string& inputPath)
{
  auto progUnit = parser.parseFile(inputPath.c_str());
  if (!progUnit)
    return false;

  return true;
}

static std::pair<size_t, size_t> performTest(CppParser& parser, const TestParam& params)
{
  size_t numInputFiles = 0;
  size_t numFailed     = 0;

  using FilePair                        = std::pair<std::string, std::string>;
  auto                     inputPathLen = params.inputPath.string().length();
  std::vector<std::string> parsingFailedFor;
  std::vector<FilePair>    diffFailedList;

  for (bfs::recursive_directory_iterator dirItr(params.inputPath); dirItr != bfs::recursive_directory_iterator();
       ++dirItr)
  {
    CppWriter cppWriter;
    bfs::path file = *dirItr;
    if (bfs::is_regular_file(file))
    {
      ++numInputFiles;
      std::cout << "CppParserTest: Parsing " << file.string() << " ...\n";
      auto      fileRelPath = file.string().substr(inputPathLen);
      bfs::path outfile     = params.outputPath / fileRelPath;
      bfs::remove(outfile);
      if (parseAndEmitFormatted(parser, file, outfile, cppWriter) && bfs::exists(outfile))
      {
        bfs::path           masfile = params.masterPath / fileRelPath;
        std::pair<int, int> diffStartInfo;
        auto                rez = compareFiles(outfile, masfile, diffStartInfo);
        if (rez == kSameFiles)
          continue;
        reportFileComparisonError(rez, outfile, masfile, diffStartInfo);
        diffFailedList.emplace_back(std::make_pair(outfile.string(), masfile.string()));
      }
      else
      {
        auto filePathStr = file.string();
        std::cerr << "Parsing failed for " << filePathStr << "\n";
        parsingFailedFor.push_back(filePathStr);
      }
      ++numFailed;
    }
  }
  if (!diffFailedList.empty())
  {
    std::cerr << "\n\n";
    std::cerr << "Comparision failure summary.\n------------------------\n";
    for (const auto& itr : diffFailedList)
    {
      std::cerr << itr.first << ' ' << itr.second << '\n';
    }
    std::cerr << "Comparision failed for " << diffFailedList.size() << " files.\n\n";
  }
  if (!parsingFailedFor.empty())
  {
    std::cerr << "\n\n";
    std::cerr << "Parsing failure summary.\n------------------------\n";
    for (const auto& s : parsingFailedFor)
    {
      std::cerr << s << '\n';
    }
    std::cerr << "Parsing failed for " << parsingFailedFor.size() << " files.\n\n";
  }

  return std::make_pair(numInputFiles, numFailed);
}

CppParser constructCppParserForTest()
{
  CppParser parser;
  parser.addKnownApiDecors({"ODRX_ABSTRACT",
                            "FIRSTDLL_EXPORT",
                            "GE_DLLEXPIMPORT",
                            "TOOLKIT_EXPORT",

                            "APIENTRY",
                            "WINGDIAPI",
                            "GLUTAPI",

                            "ADESK_NO_VTABLE",
                            "ACDBCORE2D_PORT",
                            "ACBASE_PORT",
                            "ACCORE_PORT",
                            "ACDB_PORT",
                            "ACPAL_PORT",
                            "ACAD_PORT",
                            "ACPL_PORT",
                            "ACTCUI_PORT",
                            "ADESK_DEPRECATED",
                            "DRAWBRIDGE_API",
                            "AXAUTOEXP",
                            "GX_DLLEXPIMPORT",
                            "ANAV_PORT",
                            "DRAWBRIDGE_MAC_API",
                            "ADUI_PORT",
                            "ACMPOLYGON_PORT",
                            "ACFDUI_PORT",
                            "GE_DLLDATAEXIMP",
                            "ACSYNERGY_PORT",
                            "ADESK_STDCALL",
                            "LIGHTDLLIMPEXP",
                            "SCENEDLLIMPEXP",
                            "DLLScope",

                            "_CRTIMP",

                            "SKSL_WARN_UNUSED_RESULT",
                            "SK_ALWAYS_INLINE",
                            "SK_API",
                            "SK_BEGIN_REQUIRE_DENSE",
                            "SK_WARN_UNUSED_RESULT",
                            "SK_CAPABILITY",

                            "PODOFO_DEPRECATED",
                            "PODOFO_API",

                            "WXDLLEXPORT",
                            "WXDLLIMPEXP_ADV",
                            "WXDLLIMPEXP_AUI",
                            "WXDLLIMPEXP_BASE",
                            "WXDLLIMPEXP_CORE",
                            "WXDLLIMPEXP_FWD_AUI",
                            "WXDLLIMPEXP_FWD_BASE",
                            "WXDLLIMPEXP_FWD_CORE",
                            "WXDLLIMPEXP_FWD_GL",
                            "WXDLLIMPEXP_FWD_HTML",
                            "WXDLLIMPEXP_FWD_NET",
                            "WXDLLIMPEXP_FWD_PROPGRID",
                            "WXDLLIMPEXP_FWD_RIBBON",
                            "WXDLLIMPEXP_FWD_RICHTEXT",
                            "WXDLLIMPEXP_FWD_XML",
                            "WXDLLIMPEXP_FWD_XRC",
                            "WXDLLIMPEXP_GL",
                            "WXDLLIMPEXP_HTML",
                            "WXDLLIMPEXP_MEDIA",
                            "WXDLLIMPEXP_NET",
                            "WXDLLIMPEXP_PROPGRID",
                            "WXDLLIMPEXP_QA",
                            "WXDLLIMPEXP_RIBBON",
                            "WXDLLIMPEXP_RICHTEXT",
                            "WXDLLIMPEXP_STC",
                            "WXDLLIMPEXP_WEBVIEW",
                            "WXDLLIMPEXP_XML",
                            "WXDLLIMPEXP_XRC",
                            "wxMSVC_FWD_MULTIPLE_BASES",
                            "wxDEPRECATED_CLASS_MSG",
                            "wxEXTERNC"});

  parser.addKnownMacros({"DECLARE_MESSAGE_MAP",
                         "DECLARE_DYNAMIC",
                         "ACPL_DECLARE_MEMBERS",
                         "DBSYMUTL_MAKE_GETSYMBOLID_FUNCTION",
                         "DBSYMUTL_MAKE_HASSYMBOLID_FUNCTION",
                         "DBSYMUTL_MAKE_HASSYMBOLNAME_FUNCTION",
                         "ACRX_DECLARE_MEMBERS_EXPIMP",
                         "ACRX_DECLARE_MEMBERS_ACBASE_PORT_EXPIMP",
                         "ACRX_DECLARE_MEMBERS",
                         "DBCURVE_METHODS",

                         "SK_BEGIN_REQUIRE_DENSE",
                         "SK_END_REQUIRE_DENSE",
                         "GR_MAKE_BITFIELD_CLASS_OPS",
                         "SK_C_PLUS_PLUS_BEGIN_GUARD",
                         "SK_C_PLUS_PLUS_END_GUARD",
                         "GPU_DRIVER_BUG_WORKAROUNDS",
                         "GR_MAKE_BITFIELD_OPS",
                         "SK_FLATTENABLE_HOOKS",
                         "SK_USE_FLUENT_IMAGE_FILTER_TYPES_IN_CLASS",
                         "SK_RASTER_PIPELINE_STAGES",
                         "INTERNAL_DECLARE_SET_TRACE_VALUE_INT",
                         "INTERNAL_DECLARE_SET_TRACE_VALUE",
                         "SK_RECORD_TYPES",
                         "SK_OT_BYTE_BITFIELD",
                         "SKSL_PRINTF_LIKE",
                         "ACT_AS_PTR",
                         "RECORD",
                         "GR_DECLARE_FRAGMENT_PROCESSOR_TEST",
                         "GR_DECLARE_GEOMETRY_PROCESSOR_TEST",
                         "GR_DECLARE_XP_FACTORY_TEST",
                         "DEFINE_NAMED_APPEND",
                         "SK_CALLABLE_TRAITS__CV_REF_NE_VARARGS",
                         "SK_CALLABLE_TRAITS__NE_VARARGS",
                         "SK_STDMETHODIMP_",
                         "SK_END_REQUIRE_DENSE",
                         "GR_DECL_BITFIELD_OPS_FRIENDS",
                         "SK_PRINTF_LIKE",
                         "DEFINE_OP_CLASS_ID",
                         "SHARD",
                         "SK_WHEN",

                         "PODOFO_RAISE_LOGIC_IF",

                         "va_arg",

                         "WX_DECLARE_USER_EXPORTED_BASEARRAY",
                         "WX_DEFINE_USER_EXPORTED_ARRAY_SHORT",
                         "WX_DEFINE_USER_EXPORTED_ARRAY_INT",
                         "WX_DEFINE_USER_EXPORTED_ARRAY_DOUBLE",
                         "WX_DEFINE_USER_EXPORTED_ARRAY_LONG",
                         "WX_DEFINE_USER_EXPORTED_ARRAY_PTR",
                         "WX_DEFINE_SORTED_EXPORTED_ARRAY_CMP_INT",
                         "WX_DEFINE_EXPORTED_ARRAY_PTR",
                         "WX_DEFINE_ARRAY_PTR",
                         "WX_DEFINE_VARARG_FUNC_CTOR",
                         "WX_DEFINE_USER_EXPORTED_ARRAY_SIZE_T",
                         "WX_DECLARE_LIST_WITH_DECL",
                         "WX_DECLARE_STRING_HASH_MAP_WITH_DECL",
                         "WX_DECLARE_EXPORTED_VOIDPTR_HASH_MAP",
                         "WX_DECLARE_USER_EXPORTED_OBJARRAY",
                         "WX_DECLARE_USER_EXPORTED_LIST",
                         "WX_DECLARE_OBJARRAY_WITH_DECL",
                         "WX_DECLARE_EXPORTED_HASH_MAP",
                         "WX_DECLARE_TYPE_POD",
                         "WX_DECLARE_TYPE_MOVABLE",
                         "WX_TYPE_HIERARCHY_LEVEL",
                         "wxDECLARE_EXPORTED_EVENT",
                         "wxDEPRECATED_MSG",
                         "DECLARE_VARIANT_OBJECT_EXPORTED",
                         "WX_ANY_DEFINE_CONVERTIBLE_TYPE",
                         "WX_ANY_DEFINE_CONVERTIBLE_TYPE_BASE",
                         "WX_ANY_DEFINE_SUB_TYPE",
                         "WX_ARG_NORMALIZER_FORWARD",
                         "WX_CLEAR_ARRAY",
                         "WX_CLEAR_LIST",
                         "WX_DECLARE_ABSTRACT_TYPEINFO",
                         "WX_DECLARE_ANY_VALUE_TYPE",
                         "WX_DECLARE_CONTROL_CONTAINER_BASE",
                         "WX_DECLARE_EXPORTED_HASH_MAP",
                         "WX_DECLARE_EXPORTED_LIST",
                         "WX_DECLARE_EXPORTED_OBJARRAY",
                         "WX_DECLARE_EXPORTED_VOIDPTR_HASH_MAP",
                         "WX_DECLARE_GLOBAL_CONV",
                         "WX_DECLARE_HASH_MAP",
                         "WX_DECLARE_HASH_MAP_WITH_DECL",
                         "WX_DECLARE_HASH_SET",
                         "WX_DECLARE_HASH_SET_WITH_DECL",
                         "WX_DECLARE_HASH_SET_WITH_DECL_PTR",
                         "WX_DECLARE_INPUT_CONSUMER",
                         "WX_DECLARE_LIST",
                         "WX_DECLARE_LIST_2",
                         "WX_DECLARE_LIST_3",
                         "WX_DECLARE_LIST_4",
                         "WX_DECLARE_LIST_ITER_DIFF_AND_CATEGORY",
                         "WX_DECLARE_LIST_PTR_2",
                         "WX_DECLARE_LIST_PTR_3",
                         "WX_DECLARE_LIST_WITH_DECL",
                         "WX_DECLARE_LIST_XO",
                         "WX_DECLARE_OBJARRAY",
                         "WX_DECLARE_OBJARRAY_WITH_DECL",
                         "WX_DECLARE_STRING_HASH_MAP",
                         "WX_DECLARE_STRING_HASH_MAP_WITH_DECL",
                         "WX_DECLARE_TYPEINFO_INLINE",
                         "WX_DECLARE_TYPE_IS_INT",
                         "WX_DECLARE_TYPE_MOVABLE",
                         "WX_DECLARE_TYPE_POD",
                         "WX_DECLARE_USER_EXPORTED_BASEARRAY",
                         "WX_DECLARE_USER_EXPORTED_LIST",
                         "WX_DECLARE_USER_EXPORTED_OBJARRAY",
                         "WX_DECLARE_VOIDPTR_HASH_MAP",
                         "WX_DECLARE_VOIDPTR_HASH_MAP_WITH_DECL",
                         "WX_DEFINE_ARRAY",
                         "WX_DEFINE_ARRAY_INT",
                         "WX_DEFINE_ARRAY_PTR",
                         "WX_DEFINE_ARRAY_WITH_DECL_PTR",
                         "WX_DEFINE_EXPORTED_ARRAY_PTR",
                         "WX_DEFINE_EXPORTED_TYPEARRAY",
                         "WX_DEFINE_EXPORTED_TYPEARRAY_PTR",
                         "WX_DEFINE_ITERATOR_CATEGORY",
                         "WX_DEFINE_SCANFUNC",
                         "WX_DEFINE_SORTED_EXPORTED_ARRAY_CMP_INT",
                         "WX_DEFINE_SORTED_EXPORTED_TYPEARRAY",
                         "WX_DEFINE_SORTED_EXPORTED_TYPEARRAY_CMP",
                         "WX_DEFINE_SORTED_TYPEARRAY",
                         "WX_DEFINE_SORTED_TYPEARRAY_CMP",
                         "WX_DEFINE_SORTED_USER_EXPORTED_TYPEARRAY",
                         "WX_DEFINE_SORTED_USER_EXPORTED_TYPEARRAY_CMP",
                         "WX_DEFINE_STRINGIMPL_ITERATOR",
                         "WX_DEFINE_TYPEARRAY",
                         "WX_DEFINE_TYPEARRAY_PTR",
                         "WX_DEFINE_TYPEARRAY_WITH_DECL",
                         "WX_DEFINE_TYPEARRAY_WITH_DECL_PTR",
                         "WX_DEFINE_USER_EXPORTED_ARRAY_DOUBLE",
                         "WX_DEFINE_USER_EXPORTED_ARRAY_INT",
                         "WX_DEFINE_USER_EXPORTED_ARRAY_LONG",
                         "WX_DEFINE_USER_EXPORTED_ARRAY_PTR",
                         "WX_DEFINE_USER_EXPORTED_ARRAY_SHORT",
                         "WX_DEFINE_USER_EXPORTED_ARRAY_SIZE_T",
                         "WX_DEFINE_USER_EXPORTED_TYPEARRAY",
                         "WX_DEFINE_VARARG_FUNC",
                         "WX_DEFINE_VARARG_FUNC_CTOR",
                         "WX_DEFINE_VARARG_FUNC_NOP",
                         "WX_DEFINE_VARARG_FUNC_SANS_N0",
                         "WX_DEFINE_VARARG_FUNC_VOID",
                         "WX_DELEGATE_TO_CONTROL_CONTAINER_BASE",
                         "WX_FORWARD_TO_SCROLL_HELPER",
                         "WX_FORWARD_TO_VAR_SCROLL_HELPER",
                         "WX_JOIN",
                         "WX_MAYBE_PREFIX_WITH_STRUCT",
                         "WX_MSW_DECLARE_HANDLE",
                         "WX_PG_DECLARE_ARRAYSTRING_PROPERTY_WITH_VALIDATOR",
                         "WX_PG_DECLARE_ARRAYSTRING_PROPERTY_WITH_VALIDATOR_WITH_DECL",
                         "WX_PG_DECLARE_EDITOR_WITH_DECL",
                         "WX_PG_DECLARE_PROPERTY_CLASS",
                         "WX_PG_DECLARE_VARIANT_DATA_EXPORTED",
                         "WX_PG_IMPLEMENT_ARRAYSTRING_PROPERTY_WITH_VALIDATOR",
                         "WX_PG_IMPLEMENT_PROPERTY_CLASS_PLAIN",
                         "WX_PG_IMPLEMENT_VARIANT_DATA_EQ",
                         "WX_PG_IMPLEMENT_VARIANT_DATA_EXPORTED",
                         "WX_PG_IMPLEMENT_VARIANT_DATA_EXPORTED_DUMMY_EQ",
                         "WX_PG_IMPLEMENT_VARIANT_DATA_EXPORTED_NO_EQ_NO_GETTER",
                         "WX_PG_IMPLEMENT_VARIANT_DATA_GETTER",
                         "WX_STRCMP_FUNC",
                         "WX_STR_FUNC",
                         "WX_STR_FUNC_NO_INVERT",
                         "WX_STR_ITERATOR_IMPL",
                         "WX_STRTOX_DEFINE_NULLPTR_OVERLOADS",
                         "WX_STRTOX_FUNC",
                         "WX_TYPE_HIERARCHY_LEVEL",
                         "WX_USE_THEME",
                         "WX_USE_THEME_IMPL",
                         "WX_VARARG_VFOO_IMPL",

                         "wxDECLARE_ABSTRACT_CLASS",
                         "wxDECLARE_ABSTRACT_PLUGGABLE_CLASS",
                         "wxDECLARE_ANY_TYPE",
                         "wxDECLARE_APP",
                         "wxDECLARE_CLASS",
                         "wxDECLARE_CLASS_INFO_ITERATORS",
                         "wxDECLARE_COMMON_FONT_METHODS",
                         "wxDECLARE_DYNAMIC_CLASS",
                         "wxDECLARE_DYNAMIC_CLASS_NO_ASSIGN",
                         "wxDECLARE_DYNAMIC_CLASS_NO_COPY",
                         "wxDECLARE_EVENT",
                         "wxDECLARE_EVENT_TABLE",
                         "wxDECLARE_EVENT_TABLE_ENTRY",
                         "wxDECLARE_EVENT_TABLE_TERMINATOR",
                         "wxDECLARE_EXPORTED_EVENT",
                         "wxDECLARE_EXPORTED_EVENT_ALIAS",
                         "wxDECLARE_NO_ASSIGN_CLASS",
                         "wxDECLARE_NO_COPY_CLASS",
                         "wxDECLARE_NO_COPY_TEMPLATE_CLASS",
                         "wxDECLARE_NO_COPY_TEMPLATE_CLASS_2",
                         "wxDECLARE_PLUGGABLE_CLASS",
                         "wxDECLARE_SCOPED_ARRAY",
                         "wxDECLARE_SCOPED_PTR",
                         "wxDECLARE_SYM_FUNCTION",
                         "wxDECLARE_TREELIST_EVENT",
                         "wxDECLARE_USER_EXPORTED_ABSTRACT_PLUGGABLE_CLASS",
                         "wxDECLARE_USER_EXPORTED_PLUGGABLE_CLASS",
                         "wxDECL_FOR_MINGW32_ALWAYS",
                         "wxDECL_FOR_STRICT_MINGW32",
                         "wxDEFINE_ALL_COMPARISONS",
                         "wxDEFINE_COMPARISON",
                         "wxDEFINE_COMPARISON_BY_REV",
                         "wxDEFINE_COMPARISON_REV",
                         "wxDEFINE_COMPARISONS",
                         "wxDEFINE_COMPARISONS_BY_REV",
                         "wxDEFINE_EMPTY_LOG_FUNCTION",
                         "wxDEFINE_EMPTY_LOG_FUNCTION2",
                         "wxDEFINE_EVENT",
                         "wxDEFINE_EVENT_ALIAS",
                         "wxDEFINE_FLAGS",
                         "wxDEFINE_SCOPED_ARRAY",
                         "wxDEFINE_SCOPED_PTR",
                         "wxDEFINE_SCOPED_PTR_TYPE",
                         "wxDEFINE_TIED_SCOPED_PTR_TYPE",
                         "wxDEFINE_UNICHAR_CMP_WITH_INT",
                         "wxDEFINE_UNICHAR_OPERATOR",
                         "wxDEFINE_UNICHARREF_CMP_WITH_INT",
                         "wxDEFINE_UNICHARREF_OPERATOR",
                         "wxDELETE",
                         "wxDELETEA",
                         "wxDEPRECATED",
                         "wxDEPRECATED_ACCESSOR",
                         "wxDEPRECATED_ATTR",
                         "wxDEPRECATED_BUT_USED_INTERNALLY",
                         "wxDEPRECATED_BUT_USED_INTERNALLY_INLINE",
                         "wxDEPRECATED_CONSTRUCTOR",
                         "wxDEPRECATED_INLINE",
                         "wxDEPRECATED_MSG",
                         "WX_DECLARE_VOIDPTR_HASH_MAP",
                         "wxPERSIST_DECLARE_SAVE_RESTORE_FOR",

                         "WX_DECLARE_STRING_HASH_MAP",
                         "WX_DECLARE_HASH_MAP",
                         "wxDECL_FOR_STRICT_MINGW32",
                         "wxDECL_FOR_MINGW32_ALWAYS",
                         "DECLARE_WXANY_CONVERSION",
                         "WX_FORWARD_TO_VAR_SCROLL_HELPER",
                         "wx_truncate_cast",
                         "wxFORMAT_STRING_SPECIFIER",
                         "wxDISABLED_FORMAT_STRING_SPECIFIER",
                         "WX_ARG_NORMALIZER_FORWARD",
                         "wxTLS_TYPE",
                         "WX_STR_ITERATOR_IMPL",
                         "WX_DEFINE_ITERATOR_CATEGORY",
                         "WX_DEFINE_VARARG_FUNC",
                         "wxDEFINE_ALL_COMPARISONS",
                         "DECLARE_HELP_PROVISION",
                         "wxDECLARE_EVENT_TABLE",
                         "wxDECLARE_DYNAMIC_CLASS",
                         "DECLARE_BASE_CLASS_HELP_PROVISION",
                         "WX_FORWARD_TO_SCROLL_HELPER",
                         "WX_DECLARE_GLOBAL_CONV",
                         "wxUSTRING_COMP_OPERATORS"});

  parser.addIgnorableMacros({"SkDEBUGCODE",
                             "SkDEBUGPARAMS",
                             "__bridge",
                             "__bridge_retained",
                             "API_AVAILABLE",
                             "SK_RESTRICT",
                             "DEBUG_COIN_DECLARE_PARAMS",
                             "PATH_OPS_DEBUG_T_SECT_CODE",
                             "PATH_OPS_DEBUG_T_SECT_PARAMS",
                             "SK_GUARDED_BY",
                             "SK_ACQUIRE",
                             "SK_REQUIRES",
                             "SK_RELEASE_CAPABILITY",
                             "SK_ASSERT_CAPABILITY",
                             "SK_ACQUIRE_SHARED",
                             "SK_RELEASE_SHARED_CAPABILITY",
                             "SK_BLITBWMASK_ARGS",
                             "SK_ASSERT_SHARED_CAPABILITY",

                             "PODOFO_LOCAL",
                             "PDF_SIZE_FORMAT",

                             "WXUNUSED",
                             "WXDLLIMPEXP_DATA_CORE",
                             "wxDEPRECATED_BUT_USED_INTERNALLY_INLINE",
                             "wxDEPRECATED_BUT_USED_INTERNALLY",
                             "wxTRY",
                             "wxCATCH_ALL",
                             "CHECK_PREC",
                             "WXUNUSED_UNLESS_DEBUG",
                             "wxW64",
                             "wxDEPRECATED_INLINE",
                             "FAR",
                             "EMIT",
                             "wxDEPRECATED_CONSTRUCTOR",
                             "wxDEPRECATED",
                             "wxGCC_WARNING_SUPPRESS",
                             "wxCLANG_WARNING_SUPPRESS",
                             "wxCLANG_WARNING_RESTORE",
                             "wxGCC_WARNING_RESTORE",
                             "wxSTRING_DEFAULT_CONV_ARG",
                             "WX_DECLARE_VOIDPTR_HASH_MAP_WITH_DECL"});

  parser.addUndefinedNames({"wxUSE_DATETIME", "SWIG", "CPPPARSER_DISABLED_USING_IFNDEF_PARAM_TEST", "__WXMSW__"});

  parser.addDefinedName("wxUSE_TEXTCTRL", 1);
  parser.addDefinedName("WXWIN_COMPATIBILITY_2_8", 0);
  parser.addDefinedName("WXWIN_COMPATIBILITY_3_0", 0);

  parser.addRenamedKeyword("virtual", "ADESK_SEALED_VIRTUAL");
  parser.addRenamedKeyword("virtual", "_VIRTUAL");
  parser.addRenamedKeyword("final", "ADESK_SEALED");
  parser.addRenamedKeyword("override", "ADESK_OVERRIDE");
  parser.addRenamedKeyword("override", "wxOVERRIDE");
  parser.addRenamedKeyword("const", "CONST");

  return std::move(parser);
}

int main(int argc, char** argv)
{
  CppParser parser = constructCppParserForTest();
  parser.parseEnumBodyAsBlob();

  ArgParser argParser;
  auto      optionParseResult = argParser.parse(argc, argv);
  if (optionParseResult == ArgParser::kParsingError)
  {
    argParser.emitError();
    return -1;
  }
  else if (optionParseResult == ArgParser::kParseSingleFile)
  {
    auto filePath = argParser.extractSingleFilePath();
    performParsing(parser, filePath);
  }
  else
  {
    const auto params = argParser.extractParamsForFullTest();
    const auto result = performTest(parser, params);
    if (result.second)
    {
      std::cerr << "CppParserTest: " << result.second << " tests failed out of " << result.first << ".\n";
      return 1;
    }
    std::cout << "CppParserTest: All " << result.first << " tests passed without error.\n";
  }
  return 0; // All went well.
}

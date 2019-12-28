/*
   The MIT License (MIT)

   Copyright (c) 2018 Satya Das

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

#include "cppwriter.h"
#include "cpputil.h"

#include "cppcompound-accessor.h"
#include "cppfunc-accessor.h"
#include "cppobj-accessor.h"
#include "cppvar-accessor.h"

//////////////////////////////////////////////////////////////////////////

namespace {
static void emitAttribute(std::uint32_t attr, std::ostream& stm)
{
  if (attr & kStatic)
    stm << "static ";
  else if (attr & kExtern)
    stm << "extern ";
  else if (attr & kExternC)
    stm << "extern C ";

  if (attr & kConst)
    stm << "const ";
  else if (attr & kVolatile)
    stm << "volatile ";
}

static void emitTypeModifier(const CppTypeModifier& modifier, std::ostream& stm)
{
  std::uint8_t constBit = 0;
  for (constBit = 0; constBit < modifier.ptrLevel_; ++constBit)
  {
    if (modifier.constBits_ & (1 << constBit))
      stm << " const ";
    stm << "*";
  }
  if (modifier.constBits_ & (1 << constBit))
    stm << " const";
  if (modifier.refType_ == CppRefType::kByRef)
    stm << '&';
  else if (modifier.refType_ == CppRefType::kRValRef)
    stm << "&&";
}
} // namespace

void CppWriter::emit(const CppObj* cppObj, std::ostream& stm, CppIndent indentation) const
{
  return emit(cppObj, stm, indentation, false);
}

void CppWriter::emit(const CppObj* cppObj, std::ostream& stm, CppIndent indentation, bool noNewLine) const
{
  switch (cppObj->objType_)
  {
    case CppObjType::kHashDefine:
      return emitDefine((CppDefine*) cppObj, stm);
    case CppObjType::kHashUndef:
      return emitUndef((CppUndef*) cppObj, stm);
    case CppObjType::kHashInclude:
      return emitInclude((CppInclude*) cppObj, stm);
    case CppObjType::kHashIf:
      return emitHashIf((CppHashIf*) cppObj, stm);
    case CppObjType::kHashPragma:
      return emitPragma((CppPragma*) cppObj, stm);

    case CppObjType::kVarType:
      return emitVarType((CppVarType*) cppObj, stm);
    case CppObjType::kVar:
    {
      emitVar((CppVar*) cppObj, stm, indentation);
      if (!noNewLine)
        stm << ";\n";
      return;
    }
    case CppObjType::kVarList:
      return emitVarList((CppVarList*) cppObj, stm, indentation);
    case CppObjType::kEnum:
      return emitEnum((CppEnum*) cppObj, stm, indentation);
    case CppObjType::kDocComment:
      return emitDocComment((CppDocComment*) cppObj, stm, indentation);
    case CppObjType::kUsingDecl:
      return emitUsingDecl((CppUsingDecl*) cppObj, stm, indentation);
    case CppObjType::kTypedefName:
      return emitTypedef((CppTypedefName*) cppObj, stm, indentation);
    case CppObjType::kTypedefNameList:
      return emitTypedefList((CppTypedefList*) cppObj, stm, indentation);
    case CppObjType::kCompound:
      return emitCompound((CppCompound*) cppObj, stm, indentation, !noNewLine);
    case CppObjType::kFwdClsDecl:
      return emitFwdDecl((CppFwdClsDecl*) cppObj, stm, indentation);
    case CppObjType::kFunction:
      return emitFunction((CppFunction*) cppObj, stm, indentation);
    case CppObjType::kConstructor:
      return emitConstructor((CppConstructor*) cppObj, stm, indentation);
    case CppObjType::kDestructor:
      return emitDestructor((CppDestructor*) cppObj, stm, indentation);
    case CppObjType::kTypeConverter:
      return emitTypeConverter((CppTypeConverter*) cppObj, stm, indentation);
    case CppObjType::kFunctionPtr:
      return emitFunctionPtr((CppFunctionPointer*) cppObj, stm, indentation);
    case CppObjType::kIfBlock:
      return emitIfBlock((CppIfBlock*) cppObj, stm, indentation);
    case CppObjType::kWhileBlock:
      return emitWhileBlock((CppWhileBlock*) cppObj, stm, indentation);
    case CppObjType::kDoWhileBlock:
      return emitDoBlock((CppDoWhileBlock*) cppObj, stm, indentation);
    case CppObjType::kForBlock:
      return emitForBlock((CppForBlock*) cppObj, stm, indentation);
    case CppObjType::kExpression:
      emitExpr((CppExpr*) cppObj, stm, indentation);
      if (!noNewLine)
        stm << ";\n";
      break;
    case CppObjType::kSwitchBlock:
      return emitSwitchBlock(static_cast<const CppSwitchBlock*>(cppObj), stm, indentation);
    case CppObjType::kMacroCall:
      return emitMacroCall(static_cast<const CppMacroCall*>(cppObj), stm, indentation);

    case CppObjType::kBlob:
      return emitBlob((CppBlob*) cppObj, stm);

    default:
      break;
  }
}

void CppWriter::emitDefine(const CppDefine* defObj, std::ostream& stm) const
{
  stm << '#' << preproIndent_ << "define " << defObj->name_;
  if (!defObj->defn_.empty())
    stm << '\t' << defObj->defn_;
  stm << '\n';
}

void CppWriter::emitUndef(const CppUndef* undefObj, std::ostream& stm) const
{
  stm << '#' << preproIndent_ << "undef " << undefObj->name_ << '\n';
}

void CppWriter::emitInclude(const CppInclude* includeObj, std::ostream& stm) const
{
  stm << '#' << preproIndent_ << "include " << includeObj->name_ << '\n';
}

void CppWriter::emitHashIf(const CppHashIf* hashIfObj, std::ostream& stm) const
{
  switch (hashIfObj->condType_)
  {
    case CppHashIf::kIf:
      stm << '#' << preproIndent_ << "if " << hashIfObj->cond_ << '\n';
      ++preproIndent_;
      break;

    case CppHashIf::kIfDef:
      stm << '#' << preproIndent_ << "ifdef " << hashIfObj->cond_ << '\n';
      ++preproIndent_;
      break;

    case CppHashIf::kIfNDef:
      stm << '#' << preproIndent_ << "ifndef " << hashIfObj->cond_ << '\n';
      ++preproIndent_;
      break;

    case CppHashIf::kElIf:
      stm << '#' << --preproIndent_ << "elif " << hashIfObj->cond_ << '\n';
      ++preproIndent_;
      break;

    case CppHashIf::kElse:
      stm << '#' << --preproIndent_ << "else " << hashIfObj->cond_ << '\n';
      ++preproIndent_;
      break;

    case CppHashIf::kEndIf:
      stm << '#' << --preproIndent_ << "endif\n";
      break;
  }
}

void CppWriter::emitPragma(const CppPragma* pragmaObj, std::ostream& stm) const
{
  stm << '#' << preproIndent_ << "pragma " << pragmaObj->defn_ << '\n';
}

void CppWriter::emitBlob(const CppBlob* blobObj, std::ostream& stm) const
{
  stm << blobObj->blob_;
}

void CppWriter::emitVarType(const CppVarType* varTypeObj, std::ostream& stm) const
{
  const auto attr = varTypeObj->typeAttr() | (isConst(varTypeObj) ? CppIdentifierAttrib::kConst : 0);
  emitAttribute(attr, stm);
  if (varTypeObj->compound())
    emit(varTypeObj->compound(), stm, CppIndent(), true);
  else
    stm << varTypeObj->baseType();
  const auto&           origTypeModifier = varTypeObj->typeModifier();
  const CppTypeModifier typeModifier {
    origTypeModifier.refType_, origTypeModifier.ptrLevel_, origTypeModifier.constBits_ & ~1};
  emitTypeModifier(typeModifier, stm);
}

void CppWriter::emitVar(const CppVar* varObj, std::ostream& stm, CppIndent indentation) const
{
  emitVar(varObj, stm, indentation, false);
}

void CppWriter::emitVarDecl(std::ostream& stm, const CppVarDecl& varDecl, bool skipName) const
{
  if (!skipName && !varDecl.name().empty())
    stm << varDecl.name();
  for (const auto& arrSize : varDecl.arraySizes())
  {
    stm << '[';
    if (arrSize)
      emitExpr(arrSize.get(), stm);
    stm << ']';
  }
  if (varDecl.assignType() == AssignType::kUsingEqual)
  {
    stm << " = ";
    emit(varDecl.assignValue(), stm, CppIndent(), true);
  }
  else if (varDecl.assignType() == AssignType::kUsingBracket)
  {
    stm << '(';
    if (varDecl.assignValue())
      emit(varDecl.assignValue(), stm, CppIndent(), true);
    stm << ')';
  }
  else if (varDecl.assignType() == AssignType::kUsingBraces)
  {
    stm << '{';
    if (varDecl.assignValue())
      emit(varDecl.assignValue(), stm, CppIndent(), true);
    stm << '}';
  }
}

void CppWriter::emitVar(const CppVar* varObj, std::ostream& stm, CppIndent indentation, bool skipName) const
{
  stm << indentation;
  if (!varObj->apidecor().empty())
  {
    stm << varObj->apidecor() << ' ';
  }
  emitVarType(varObj->varType(), stm);
  if (!skipName && !varObj->name().empty())
    stm << ' ';
  emitVarDecl(stm, varObj->varDecl(), skipName);
}

void CppWriter::emitVarList(const CppVarList* varListObj,
                            std::ostream&     stm,
                            CppIndent         indentation /* = CppIndent()*/) const
{
  stm << indentation;
  emitAttribute(varListObj->typeAttr_, stm);
  stm << varListObj->baseType_ << ' ';
  auto& varDeclList = varListObj->varDeclList();
  for (size_t i = 0; i < varDeclList.size(); ++i)
  {
    if (i > 0)
      stm << ", ";
    const auto& decl = varDeclList[i];
    emitTypeModifier(decl, stm);
    emitVarDecl(stm, decl, false);
  }

  stm << ";\n";
}

void CppWriter::emitEnum(const CppEnum* enmObj, std::ostream& stm, CppIndent indentation /* = CppIndent()*/) const
{
  stm << indentation << "enum";
  if (enmObj->isClass_)
    stm << " class";
  if (!enmObj->name_.empty())
    stm << ' ' << enmObj->name_;
  if (!enmObj->underlyingType_.empty())
    stm << " : " << enmObj->underlyingType_;
  if (enmObj->itemList_)
  {
    stm << '\n';
    stm << indentation++ << "{\n";
    for (auto enmItem : *(enmObj->itemList_))
    {
      if (enmItem->name_.empty())
      {
        emit(enmItem->val_.get(), stm, indentation);
      }
      else
      {
        stm << indentation << enmItem->name_;
        if (enmItem->val_ && isExpr(enmItem->val_.get()))
        {
          auto* expr = static_cast<CppExpr*>(enmItem->val_.get());
          stm << " = ";
          emitExpr(expr, stm);
        }
        if (enmItem != enmObj->itemList_->back())
          stm << ",\n";
        else
          stm << '\n';
      }
    }
    stm << --indentation << "}";
  }
  stm << ";\n";
}

void CppWriter::emitTypedef(const CppTypedefName* typedefName,
                            std::ostream&         stm,
                            CppIndent             indentation /* = CppIndent()*/) const
{
  stm << indentation << "typedef ";
  emitVar(typedefName->var_.get(), stm);
  stm << ";\n";
}

void CppWriter::emitUsingDecl(const CppUsingDecl* usingDecl,
                              std::ostream&       stm,
                              CppIndent           indentation /* = CppIndent()*/) const
{
  if (usingDecl->templateParamList())
    emitTemplSpec(usingDecl->templateParamList(), stm, indentation);
  stm << indentation << "using " << usingDecl->name_;
  if (usingDecl->cppObj_)
  {
    stm << " = ";
    emit(usingDecl->cppObj_.get(), stm);
  }
  stm << ";\n";
}

void CppWriter::emitTypedefList(const CppTypedefList* typedefList,
                                std::ostream&         stm,
                                CppIndent             indentation /* = CppIndent()*/) const
{
  stm << indentation << "typedef ";
  emitVarList(typedefList->varList_.get(), stm);
}

void CppWriter::emitFwdDecl(const CppFwdClsDecl* fwdDeclObj,
                            std::ostream&        stm,
                            CppIndent            indentation /* = CppIndent()*/) const
{
  if (fwdDeclObj->templateParamList())
    emitTemplSpec(fwdDeclObj->templateParamList(), stm, indentation);
  stm << indentation;
  if (fwdDeclObj->attr() & kFriend)
    stm << "friend ";
  stm << fwdDeclObj->cmpType_ << ' ' << fwdDeclObj->name_ << ";\n";
}

void CppWriter::emitMacroCall(const CppMacroCall* macroCallObj,
                              std::ostream&       stm,
                              CppIndent           indentation /* = CppIndent()*/) const
{
  stm << indentation << macroCallObj->macroCall_ << '\n';
}

void CppWriter::emitTemplSpec(const CppTemplateParamList* templSpec, std::ostream& stm, CppIndent indentation) const
{
  stm << indentation << "template <";
  if (templSpec)
  {
    const char* sep = "";
    for (auto& param : *templSpec)
    {
      stm << sep;
      if (param->paramType_)
      {
        emitVarType(param->paramType_.get(), stm);
        stm << ' ';
      }
      else
      {
        stm << "typename ";
      }
      stm << param->paramName_;
      if (param->defaultArg())
      {
        stm << " = ";
        emit(param->defaultArg(), stm, CppIndent(), true);
      }
      sep = ", ";
    }
  }
  stm << ">\n";
}

void CppWriter::emitCompound(const CppCompound* compoundObj,
                             std::ostream&      stm,
                             CppIndent          indentation,
                             bool               emitNewLine) const
{
  if (isNamespaceLike(compoundObj))
  {
    if (compoundObj->templateParamList())
    {
      emitTemplSpec(compoundObj->templateParamList(), stm, indentation);
    }
    stm << indentation << compoundObj->compoundType() << ' ';
    if (!compoundObj->apidecor().empty())
      stm << compoundObj->apidecor() << ' ';
    stm << compoundObj->name();
  }
  if (compoundObj->inheritanceList())
  {
    ++indentation;
    char sep = ':';
    stm << ' ';
    for (CppInheritanceList::const_iterator inhItr = compoundObj->inheritanceList()->begin();
         inhItr != compoundObj->inheritanceList()->end();
         ++inhItr)
    {
      stm << sep << ' ' << inhItr->inhType << ' ' << inhItr->baseName;
      sep = ',';
    }
    --indentation;
  }
  if (isNamespaceLike(compoundObj))
    stm << '\n' << indentation++ << "{\n";
  else if (compoundObj->compoundType() == CppCompoundType::kExternCBlock)
    stm << indentation++ << "extern \"C\" {\n";

  CppAccessType lastAccessType = CppAccessType::kUnknown;
  forEachMember(compoundObj, [&](const CppObj* memObj) {
    if (isClassLike(compoundObj) && memObj->accessType_ != CppAccessType::kUnknown
        && lastAccessType != memObj->accessType_)
    {
      stm << --indentation << memObj->accessType_ << ':' << '\n';
      lastAccessType = memObj->accessType_;
      ++indentation;
    }
    emit(memObj, stm, indentation);

    return false;
  });

  if (isNamespaceLike(compoundObj))
  {
    stm << --indentation;
    stm << '}';
    if (emitNewLine)
    {
      if (isClassLike(compoundObj))
        stm << ';';
      stm << '\n';
    }
  }
  else if (compoundObj->compoundType() == CppCompoundType::kExternCBlock)
    stm << indentation << "}\n";
}

void CppWriter::emitParamList(const CppParamVector* paramListObj, std::ostream& stm) const
{
  emitParamList(paramListObj, stm, false);
}

void CppWriter::emitParamList(const CppParamVector* paramListObj, std::ostream& stm, bool skipParamName) const
{
  for (auto prmItr = paramListObj->begin(); prmItr != paramListObj->end(); ++prmItr)
  {
    if (prmItr != paramListObj->begin())
      stm << ", ";
    auto& param = *prmItr;
    switch (param->objType_)
    {
      case CppObjType::kVar:
        emitVar(static_cast<const CppVar*>(param.get()), stm, CppIndent(), skipParamName);
        break;
      case CppObjType::kFunctionPtr:
        emitFunctionPtr(static_cast<const CppFunctionPointer*>(param.get()), stm, skipParamName);
        break;
      default:
        assert(false);
    }
  }
}

void CppWriter::emitFunction(const CppFunction* funcObj,
                             std::ostream&      stm,
                             CppIndent          indentation,
                             bool               skipName,
                             bool               skipParamName) const
{
  if (funcObj->templateParamList())
    emitTemplSpec(funcObj->templateParamList(), stm, indentation);

  if ((funcObj->attr() & (kFuncParam | kTypedef)) == 0)
    stm << indentation;
  if (!funcObj->decor1().empty())
    stm << funcObj->decor1() << ' ';
  if (funcObj->attr() & kStatic)
    stm << "static ";
  else if (funcObj->attr() & kExtern)
    stm << "extern ";
  else if (funcObj->attr() & kVirtual)
    stm << "virtual ";
  else if (funcObj->attr() & kInline)
    stm << "inline ";
  else if (funcObj->attr() & kExplicit)
    stm << "explicit ";
  else if (funcObj->attr() & kFriend)
    stm << "friend ";
  emitVarType(funcObj->retType_.get(), stm);
  if (funcObj->objType_ == CppObjType::kFunctionPtr)
    stm << " (";
  else
    stm << ' ';
  if (!funcObj->decor2().empty())
    stm << funcObj->decor2() << ' ';
  if (funcObj->objType_ == CppObjType::kFunctionPtr)
  {
    stm << '*';
    if (!skipName)
      stm << funcObj->name_ << ") ";
  }
  else if (!skipName)
  {
    stm << funcObj->name_;
  }
  stm << '(';
  if (funcObj->params())
    emitParamList(funcObj->params(), stm, skipParamName);
  stm << ')';

  if ((funcObj->attr() & kConst) == kConst)
    stm << " const";
  if ((funcObj->attr() & kPureVirtual) == kPureVirtual)
    stm << " = 0";
  else if ((funcObj->attr() & kOverride) == kOverride)
    stm << " override";
  else if ((funcObj->attr() & kFinal) == kFinal)
    stm << " final";

  if (!skipParamName && funcObj->defn() && (getEmittingType() != kHeader))
  {
    stm << '\n' << indentation++ << "{\n";
    emitCompound(funcObj->defn(), stm, indentation);
    stm << --indentation << "}\n";
  }
  else if ((funcObj->attr() & kFuncParam) == 0)
  {
    stm << ";\n";
  }
}

void CppWriter::emitFunction(const CppFunction* funcObj, std::ostream& stm, CppIndent indentation) const
{
  return emitFunction(funcObj, stm, indentation, false, false);
}

void CppWriter::emitFunctionPtr(const CppFunctionPointer* funcPtrObj,
                                std::ostream&             stm,
                                CppIndent                 indentation /* = CppIndent()*/) const
{
  if (funcPtrObj->attr() & kTypedef)
    stm << indentation << "typedef ";
  emitFunction((CppFunction*) funcPtrObj, stm, indentation);
}

void CppWriter::emitConstructor(const CppConstructor* ctorObj,
                                std::ostream&         stm,
                                CppIndent             indentation,
                                bool                  skipParamName) const
{
  if (ctorObj->templateParamList())
  {
    emitTemplSpec(ctorObj->templateParamList(), stm, indentation);
  }
  stm << indentation;
  if (!ctorObj->decor1().empty())
    stm << ctorObj->decor1() << ' ';
  if (ctorObj->attr() & kInline)
    stm << "inline ";
  else if (ctorObj->attr() & kExplicit)
    stm << "explicit ";
  stm << ctorObj->name_;
  stm << '(';
  if (ctorObj->params())
    emitParamList(ctorObj->params(), stm, skipParamName);
  stm << ')';
  if (!skipParamName && ctorObj->memInitList_)
  {
    char sep = ':';
    ++indentation;
    for (CppMemInitList::const_iterator memInitItr = ctorObj->memInitList_->begin();
         memInitItr != ctorObj->memInitList_->end();
         ++memInitItr)
    {
      stm << '\n';
      stm << indentation << sep << ' ' << memInitItr->first << '(';
      emitExpr(memInitItr->second, stm);
      stm << ')';
      sep = ',';
    }
    --indentation;
  }
  if (!skipParamName && ctorObj->defn())
  {
    stm << '\n' << indentation++ << "{\n";
    emitCompound(ctorObj->defn(), stm, indentation);
    stm << --indentation << "}\n";
  }
  else
  {
    if (isDeleted(ctorObj))
      stm << " = delete";
    stm << ";\n";
  }
}

void CppWriter::emitConstructor(const CppConstructor* ctorObj, std::ostream& stm, CppIndent indentation) const
{
  emitConstructor(ctorObj, stm, indentation, false);
}

void CppWriter::emitDestructor(const CppDestructor* dtorObj,
                               std::ostream&        stm,
                               CppIndent            indentation /* = CppIndent()*/) const
{
  if (dtorObj->templateParamList())
    emitTemplSpec(dtorObj->templateParamList(), stm, indentation);
  stm << indentation;
  if (!dtorObj->decor1().empty())
    stm << dtorObj->decor1() << ' ';
  if (dtorObj->attr() & kInline)
    stm << "inline ";
  else if (dtorObj->attr() & kExplicit)
    stm << "explicit ";
  else if (dtorObj->attr() & kVirtual)
    stm << "virtual ";
  stm << dtorObj->name_ << "()";

  if (dtorObj->defn())
  {
    stm << '\n' << indentation++ << "{\n";
    emitCompound(dtorObj->defn(), stm, indentation);
    stm << --indentation << "}\n";
  }
  else
  {
    stm << ";\n";
  }
}

void CppWriter::emitTypeConverter(const CppTypeConverter* typeConverterObj,
                                  std::ostream&           stm,
                                  CppIndent               indentation) const
{
  if (typeConverterObj->templateParamList())
    emitTemplSpec(typeConverterObj->templateParamList(), stm, indentation);
  stm << indentation << "operator ";
  emitVarType(typeConverterObj->to_.get(), stm);
  stm << "()";
  if (typeConverterObj->attr() & kConst)
    stm << " const";
  if (typeConverterObj->defn())
  {
    stm << '\n';
    stm << indentation << "{\n";
    ++indentation;
    emitCompound(typeConverterObj->defn(), stm, indentation);
    --indentation;
    stm << indentation << "}\n";
  }
  else
  {
    stm << ";\n";
  }
}

void CppWriter::emitDocComment(const CppDocComment* docCommentObj,
                               std::ostream&        stm,
                               CppIndent            indentation /* = CppIndent()*/) const
{
  stm << docCommentObj->doc_ << '\n';
}

inline void emitOperator(std::ostream& stm, CppOperator op)
{
  switch (op)
  {
    case kUnaryMinus:
      stm << '-';
      break;
    case kBitToggle:
      stm << '~';
      break;
    case kLogNot:
      stm << '!';
      break;
    case kDerefer:
      stm << '*';
      break;
    case kRefer:
      stm << '&';
      break;
    case kPreIncrement:
    case kPostIncrement:
      stm << "++";
      break;
    case kPreDecrement:
    case kPostDecrement:
      stm << "--";
      break;
    case kPlus:
      stm << '+';
      break;
    case kMinus:
      stm << '-';
      break;
    case kMul:
      stm << '*';
      break;
    case kDiv:
      stm << '/';
      break;
    case kPercent:
      stm << '%';
      break;
    case kAnd:
      stm << "&&";
      break;
    case kOr:
      stm << "||";
      break;
    case kBitAnd:
      stm << '&';
      break;
    case kBitOr:
      stm << '|';
      break;
    case kXor:
      stm << '^';
      break;
    case kEqual:
      stm << '=';
      break;
    case kLess:
      stm << '<';
      break;
    case kGreater:
      stm << '>';
      break;
    case kPlusEqual:
      stm << "+=";
      break;
    case kMinusEqual:
      stm << "-=";
      break;
    case kMulEqual:
      stm << "*=";
      break;
    case kDivEqual:
      stm << "/=";
      break;
    case kPerEqual:
      stm << "%=";
      break;
    case kXorEqual:
      stm << "^=";
      break;
    case kAndEqual:
      stm << "&=";
      break;
    case kOrEqual:
      stm << "|=";
      break;
    case kLeftShift:
      stm << "<<";
      break;
    case kRightShift:
      stm << ">>";
      break;
    case kLShiftEqual:
      stm << "<<=";
      break;
    case kRShiftEqual:
      stm << ">>=";
      break;
    case kCmpEqual:
      stm << "==";
      break;
    case kNotEqual:
      stm << "!=";
      break;
    case kLessEqual:
      stm << "<=";
      break;
    case kGreaterEqual:
      stm << ">=";
      break;
    case k3WayCmp:
      stm << "<=>";
      break;
    case kComma:
      stm << ',';
      break;
    case kDot:
      stm << '.';
      break;
    case kArrow:
      stm << "->";
      break;
    case kArrowStar:
      stm << "->*";
      break;

    default:
      break;
  }
}

void CppWriter::emitExprAtom(const CppExprAtom& exprAtm,
                             std::ostream&      stm,
                             CppIndent          indentation /*= CppIndent()*/) const
{
  switch (exprAtm.type)
  {
    case CppExprAtom::kAtom:
      stm << *(exprAtm.atom);
      break;
    case CppExprAtom::kExpr:
      emitExpr(exprAtm.expr, stm);
      break;
    case CppExprAtom::kVarType:
      emitVarType(exprAtm.varType, stm);

    default:
      break;
  }
}

void CppWriter::emitExpr(const CppExpr* exprObj, std::ostream& stm, CppIndent indentation /*= CppIndent()*/) const
{
  if (exprObj == NULL)
    return;
  stm << indentation;
  if (exprObj->flags_ & CppExpr::kReturn)
    stm << "return ";
  if (exprObj->flags_ & CppExpr::kThrow)
    stm << "throw ";
  if (exprObj->flags_ & CppExpr::kInitializer)
    stm << "{";
  if (exprObj->flags_ & CppExpr::kBracketed)
    stm << '(';
  if (exprObj->flags_ & CppExpr::kNew)
    stm << "new ";
  if (exprObj->flags_ & CppExpr::kSizeOf)
    stm << "sizeof(";
  else if (exprObj->flags_ & CppExpr::kDelete)
    stm << "delete ";
  else if (exprObj->flags_ & CppExpr::kDeleteArray)
    stm << "delete[] ";
  if (exprObj->oper_ == kNone)
  {
    emitExprAtom(exprObj->expr1_, stm);
  }
  else if (exprObj->oper_ > kUnariPrefixOperatorStart && exprObj->oper_ < kUnariSufixOperatorStart)
  {
    emitOperator(stm, exprObj->oper_);
    emitExprAtom(exprObj->expr1_, stm);
  }
  else if (exprObj->oper_ > kUnariSufixOperatorStart && exprObj->oper_ < kBinaryOperatorStart)
  {
    emitExprAtom(exprObj->expr1_, stm);
    emitOperator(stm, exprObj->oper_);
  }
  else if (exprObj->oper_ > kBinaryOperatorStart && exprObj->oper_ < kDerefOperatorStart)
  {
    emitExprAtom(exprObj->expr1_, stm);
    if (exprObj->oper_ != kComma)
      stm << ' ';
    emitOperator(stm, exprObj->oper_);
    stm << ' ';
    emitExprAtom(exprObj->expr2_, stm);
  }
  else if (exprObj->oper_ > kDerefOperatorStart && exprObj->oper_ < kSpecialOperations)
  {
    emitExprAtom(exprObj->expr1_, stm);
    emitOperator(stm, exprObj->oper_);
    emitExprAtom(exprObj->expr2_, stm);
  }
  else if (exprObj->oper_ == kFunctionCall)
  {
    emitExprAtom(exprObj->expr1_, stm);
    stm << '(';
    emitExprAtom(exprObj->expr2_, stm);
    stm << ')';
  }
  else if (exprObj->oper_ == kArrayElem)
  {
    emitExprAtom(exprObj->expr1_, stm);
    stm << '[';
    emitExprAtom(exprObj->expr2_, stm);
    stm << ']';
  }
  else if (exprObj->oper_ == kCStyleCast)
  {
    stm << '(';
    emitExprAtom(exprObj->expr1_, stm);
    stm << ") ";
    emitExprAtom(exprObj->expr2_, stm);
  }
  else if (exprObj->oper_ >= kConstCast && exprObj->oper_ <= kReinterpretCast)
  {
    if (exprObj->oper_ == kConstCast)
      stm << "const_cast";
    else if (exprObj->oper_ == kStaticCast)
      stm << "static_cast";
    else if (exprObj->oper_ == kDynamicCast)
      stm << "dynamic_cast";
    else if (exprObj->oper_ == kReinterpretCast)
      stm << "reinterpret_cast";
    stm << '<';
    emitExprAtom(exprObj->expr1_, stm);
    stm << ">(";
    emitExprAtom(exprObj->expr2_, stm);
    stm << ')';
  }
  else if (exprObj->oper_ == kTertiaryOperator)
  {
    emitExprAtom(exprObj->expr1_, stm);
    stm << " ? ";
    emitExprAtom(exprObj->expr2_, stm);
    stm << " : ";
    emitExprAtom(exprObj->expr3_, stm);
  }

  if (exprObj->flags_ & CppExpr::kBracketed)
    stm << ')';
  if (exprObj->flags_ & CppExpr::kInitializer)
    stm << "}";
  if (exprObj->flags_ & CppExpr::kSizeOf)
    stm << ')';
}

void CppWriter::emitIfBlock(const CppIfBlock* ifBlock, std::ostream& stm, CppIndent indentation) const
{
  stm << indentation;
  stm << "if (";
  emit(ifBlock->cond_.get(), stm, CppIndent(), true);
  stm << ")\n";
  stm << indentation << "{\n";
  ++indentation;
  if (ifBlock->body_)
    emit(ifBlock->body_.get(), stm, indentation);
  --indentation;
  stm << indentation << "}\n";
  if (ifBlock->elsePart())
  {
    stm << indentation << "else \n";
    stm << indentation << "{\n";
    ++indentation;
    emit(ifBlock->elsePart(), stm, indentation);
    --indentation;
    stm << indentation << "}\n";
  }
}

void CppWriter::emitWhileBlock(const CppWhileBlock* whileBlock, std::ostream& stm, CppIndent indentation) const
{
  stm << indentation;
  stm << "while (";
  emit(whileBlock->cond_.get(), stm, CppIndent(), true);
  stm << ")\n";
  stm << indentation << "{\n";
  ++indentation;
  if (whileBlock->body_)
    emit(whileBlock->body_.get(), stm, indentation);
  --indentation;
  stm << indentation << "}\n";
}

void CppWriter::emitDoBlock(const CppDoWhileBlock* doBlock, std::ostream& stm, CppIndent indentation) const
{
  stm << indentation << "do\n";
  stm << indentation << "{\n";
  ++indentation;
  if (doBlock->body_)
    emit(doBlock->body_.get(), stm, indentation);
  --indentation;
  stm << indentation << "} while (";
  emit(doBlock->cond_.get(), stm, CppIndent(), true);
  stm << ");\n";
}

void CppWriter::emitForBlock(const CppForBlock* forBlock, std::ostream& stm, CppIndent indentation) const
{
  stm << indentation << "for (";
  if (forBlock->start_.get())
    emit(forBlock->start_.get(), stm, CppIndent(), true);
  stm << ';';
  if (forBlock->stop_)
  {
    stm << ' ';
    emitExpr(forBlock->stop_.get(), stm);
  }
  stm << ';';
  if (forBlock->step_)
  {
    stm << ' ';
    emitExpr(forBlock->step_.get(), stm);
  }
  stm << ")\n";
  stm << indentation << "{\n";
  ++indentation;
  if (forBlock->body_)
    emit(forBlock->body_.get(), stm, indentation);
  --indentation;
  stm << indentation << "}\n";
}

void CppWriter::emitSwitchBlock(const CppSwitchBlock* switchBlock, std::ostream& stm, CppIndent indentation) const
{
  stm << indentation << "switch(";
  emitExpr(switchBlock->cond_.get(), stm);
  stm << ")\n";
  stm << indentation++ << "{\n";
  for (const auto& caseStmt : *(switchBlock->body_))
  {
    if (caseStmt.case_)
    {
      stm << indentation++ << "case ";
      emitExpr(caseStmt.case_.get(), stm);
      stm << ":\n";
    }
    else
    {
      stm << "default:\n";
    }
    if (caseStmt.body_)
      emitCompound(caseStmt.body_.get(), stm, indentation);
    --indentation;
  }
  stm << --indentation << "}\n";
}

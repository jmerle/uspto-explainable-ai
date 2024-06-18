
// Generated from src/uspto/whoosh/Whoosh.g4 by ANTLR 4.13.1

#pragma once


#include "antlr4-runtime.h"
#include "WhooshParser.h"


namespace whoosh {

/**
 * This class defines an abstract visitor for a parse tree
 * produced by WhooshParser.
 */
class  WhooshVisitor : public antlr4::tree::AbstractParseTreeVisitor {
public:

  /**
   * Visit parse trees produced by WhooshParser.
   */
    virtual std::any visitTerm(WhooshParser::TermContext *context) = 0;

    virtual std::any visitWrappedExpr(WhooshParser::WrappedExprContext *context) = 0;

    virtual std::any visitNotExpr(WhooshParser::NotExprContext *context) = 0;

    virtual std::any visitTermExpr(WhooshParser::TermExprContext *context) = 0;

    virtual std::any visitOrExpr(WhooshParser::OrExprContext *context) = 0;

    virtual std::any visitXorExpr(WhooshParser::XorExprContext *context) = 0;

    virtual std::any visitAndExpr(WhooshParser::AndExprContext *context) = 0;


};

}  // namespace whoosh

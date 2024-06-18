
// Generated from src/uspto/whoosh/Whoosh.g4 by ANTLR 4.13.1

#pragma once


#include "antlr4-runtime.h"
#include "WhooshVisitor.h"


namespace whoosh {

/**
 * This class provides an empty implementation of WhooshVisitor, which can be
 * extended to create a visitor which only needs to handle a subset of the available methods.
 */
class  WhooshBaseVisitor : public WhooshVisitor {
public:

  virtual std::any visitTerm(WhooshParser::TermContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitWrappedExpr(WhooshParser::WrappedExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitNotExpr(WhooshParser::NotExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitTermExpr(WhooshParser::TermExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitOrExpr(WhooshParser::OrExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitXorExpr(WhooshParser::XorExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAndExpr(WhooshParser::AndExprContext *ctx) override {
    return visitChildren(ctx);
  }


};

}  // namespace whoosh

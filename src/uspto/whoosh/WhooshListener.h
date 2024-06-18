
// Generated from src/uspto/whoosh/Whoosh.g4 by ANTLR 4.13.1

#pragma once


#include "antlr4-runtime.h"
#include "WhooshParser.h"


namespace whoosh {

/**
 * This interface defines an abstract listener for a parse tree produced by WhooshParser.
 */
class  WhooshListener : public antlr4::tree::ParseTreeListener {
public:

  virtual void enterTerm(WhooshParser::TermContext *ctx) = 0;
  virtual void exitTerm(WhooshParser::TermContext *ctx) = 0;

  virtual void enterWrappedExpr(WhooshParser::WrappedExprContext *ctx) = 0;
  virtual void exitWrappedExpr(WhooshParser::WrappedExprContext *ctx) = 0;

  virtual void enterNotExpr(WhooshParser::NotExprContext *ctx) = 0;
  virtual void exitNotExpr(WhooshParser::NotExprContext *ctx) = 0;

  virtual void enterTermExpr(WhooshParser::TermExprContext *ctx) = 0;
  virtual void exitTermExpr(WhooshParser::TermExprContext *ctx) = 0;

  virtual void enterOrExpr(WhooshParser::OrExprContext *ctx) = 0;
  virtual void exitOrExpr(WhooshParser::OrExprContext *ctx) = 0;

  virtual void enterXorExpr(WhooshParser::XorExprContext *ctx) = 0;
  virtual void exitXorExpr(WhooshParser::XorExprContext *ctx) = 0;

  virtual void enterAndExpr(WhooshParser::AndExprContext *ctx) = 0;
  virtual void exitAndExpr(WhooshParser::AndExprContext *ctx) = 0;


};

}  // namespace whoosh

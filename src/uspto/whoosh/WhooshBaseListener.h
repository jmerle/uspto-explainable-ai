
// Generated from src/uspto/whoosh/Whoosh.g4 by ANTLR 4.13.1

#pragma once


#include "antlr4-runtime.h"
#include "WhooshListener.h"


namespace whoosh {

/**
 * This class provides an empty implementation of WhooshListener,
 * which can be extended to create a listener which only needs to handle a subset
 * of the available methods.
 */
class  WhooshBaseListener : public WhooshListener {
public:

  virtual void enterTerm(WhooshParser::TermContext * /*ctx*/) override { }
  virtual void exitTerm(WhooshParser::TermContext * /*ctx*/) override { }

  virtual void enterWrappedExpr(WhooshParser::WrappedExprContext * /*ctx*/) override { }
  virtual void exitWrappedExpr(WhooshParser::WrappedExprContext * /*ctx*/) override { }

  virtual void enterNotExpr(WhooshParser::NotExprContext * /*ctx*/) override { }
  virtual void exitNotExpr(WhooshParser::NotExprContext * /*ctx*/) override { }

  virtual void enterTermExpr(WhooshParser::TermExprContext * /*ctx*/) override { }
  virtual void exitTermExpr(WhooshParser::TermExprContext * /*ctx*/) override { }

  virtual void enterOrExpr(WhooshParser::OrExprContext * /*ctx*/) override { }
  virtual void exitOrExpr(WhooshParser::OrExprContext * /*ctx*/) override { }

  virtual void enterXorExpr(WhooshParser::XorExprContext * /*ctx*/) override { }
  virtual void exitXorExpr(WhooshParser::XorExprContext * /*ctx*/) override { }

  virtual void enterAndExpr(WhooshParser::AndExprContext * /*ctx*/) override { }
  virtual void exitAndExpr(WhooshParser::AndExprContext * /*ctx*/) override { }


  virtual void enterEveryRule(antlr4::ParserRuleContext * /*ctx*/) override { }
  virtual void exitEveryRule(antlr4::ParserRuleContext * /*ctx*/) override { }
  virtual void visitTerminal(antlr4::tree::TerminalNode * /*node*/) override { }
  virtual void visitErrorNode(antlr4::tree::ErrorNode * /*node*/) override { }

};

}  // namespace whoosh

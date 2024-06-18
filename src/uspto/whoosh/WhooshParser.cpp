
// Generated from src/uspto/whoosh/Whoosh.g4 by ANTLR 4.13.1


#include "WhooshListener.h"
#include "WhooshVisitor.h"

#include "WhooshParser.h"


using namespace antlrcpp;
using namespace whoosh;

using namespace antlr4;

namespace {

struct WhooshParserStaticData final {
  WhooshParserStaticData(std::vector<std::string> ruleNames,
                        std::vector<std::string> literalNames,
                        std::vector<std::string> symbolicNames)
      : ruleNames(std::move(ruleNames)), literalNames(std::move(literalNames)),
        symbolicNames(std::move(symbolicNames)),
        vocabulary(this->literalNames, this->symbolicNames) {}

  WhooshParserStaticData(const WhooshParserStaticData&) = delete;
  WhooshParserStaticData(WhooshParserStaticData&&) = delete;
  WhooshParserStaticData& operator=(const WhooshParserStaticData&) = delete;
  WhooshParserStaticData& operator=(WhooshParserStaticData&&) = delete;

  std::vector<antlr4::dfa::DFA> decisionToDFA;
  antlr4::atn::PredictionContextCache sharedContextCache;
  const std::vector<std::string> ruleNames;
  const std::vector<std::string> literalNames;
  const std::vector<std::string> symbolicNames;
  const antlr4::dfa::Vocabulary vocabulary;
  antlr4::atn::SerializedATNView serializedATN;
  std::unique_ptr<antlr4::atn::ATN> atn;
};

::antlr4::internal::OnceFlag whooshParserOnceFlag;
#if ANTLR4_USE_THREAD_LOCAL_CACHE
static thread_local
#endif
WhooshParserStaticData *whooshParserStaticData = nullptr;

void whooshParserInitialize() {
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  if (whooshParserStaticData != nullptr) {
    return;
  }
#else
  assert(whooshParserStaticData == nullptr);
#endif
  auto staticData = std::make_unique<WhooshParserStaticData>(
    std::vector<std::string>{
      "term", "expr"
    },
    std::vector<std::string>{
      "", "':'", "'('", "')'", "'OR'", "'AND'", "'XOR'", "'NOT'"
    },
    std::vector<std::string>{
      "", "", "", "", "", "", "", "", "WS", "CATEGORY", "TOKEN"
    }
  );
  static const int32_t serializedATNSegment[] = {
  	4,1,10,35,2,0,7,0,2,1,7,1,1,0,1,0,1,0,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  	1,1,3,1,17,8,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,5,1,30,8,1,
  	10,1,12,1,33,9,1,1,1,0,1,2,2,0,2,0,0,38,0,4,1,0,0,0,2,16,1,0,0,0,4,5,
  	5,9,0,0,5,6,5,1,0,0,6,7,5,10,0,0,7,1,1,0,0,0,8,9,6,1,-1,0,9,17,3,0,0,
  	0,10,11,5,2,0,0,11,12,3,2,1,0,12,13,5,3,0,0,13,17,1,0,0,0,14,15,5,7,0,
  	0,15,17,3,2,1,2,16,8,1,0,0,0,16,10,1,0,0,0,16,14,1,0,0,0,17,31,1,0,0,
  	0,18,19,10,5,0,0,19,20,5,4,0,0,20,30,3,2,1,6,21,22,10,4,0,0,22,23,5,5,
  	0,0,23,30,3,2,1,5,24,25,10,3,0,0,25,26,5,6,0,0,26,30,3,2,1,4,27,28,10,
  	1,0,0,28,30,3,2,1,2,29,18,1,0,0,0,29,21,1,0,0,0,29,24,1,0,0,0,29,27,1,
  	0,0,0,30,33,1,0,0,0,31,29,1,0,0,0,31,32,1,0,0,0,32,3,1,0,0,0,33,31,1,
  	0,0,0,3,16,29,31
  };
  staticData->serializedATN = antlr4::atn::SerializedATNView(serializedATNSegment, sizeof(serializedATNSegment) / sizeof(serializedATNSegment[0]));

  antlr4::atn::ATNDeserializer deserializer;
  staticData->atn = deserializer.deserialize(staticData->serializedATN);

  const size_t count = staticData->atn->getNumberOfDecisions();
  staticData->decisionToDFA.reserve(count);
  for (size_t i = 0; i < count; i++) { 
    staticData->decisionToDFA.emplace_back(staticData->atn->getDecisionState(i), i);
  }
  whooshParserStaticData = staticData.release();
}

}

WhooshParser::WhooshParser(TokenStream *input) : WhooshParser(input, antlr4::atn::ParserATNSimulatorOptions()) {}

WhooshParser::WhooshParser(TokenStream *input, const antlr4::atn::ParserATNSimulatorOptions &options) : Parser(input) {
  WhooshParser::initialize();
  _interpreter = new atn::ParserATNSimulator(this, *whooshParserStaticData->atn, whooshParserStaticData->decisionToDFA, whooshParserStaticData->sharedContextCache, options);
}

WhooshParser::~WhooshParser() {
  delete _interpreter;
}

const atn::ATN& WhooshParser::getATN() const {
  return *whooshParserStaticData->atn;
}

std::string WhooshParser::getGrammarFileName() const {
  return "Whoosh.g4";
}

const std::vector<std::string>& WhooshParser::getRuleNames() const {
  return whooshParserStaticData->ruleNames;
}

const dfa::Vocabulary& WhooshParser::getVocabulary() const {
  return whooshParserStaticData->vocabulary;
}

antlr4::atn::SerializedATNView WhooshParser::getSerializedATN() const {
  return whooshParserStaticData->serializedATN;
}


//----------------- TermContext ------------------------------------------------------------------

WhooshParser::TermContext::TermContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* WhooshParser::TermContext::CATEGORY() {
  return getToken(WhooshParser::CATEGORY, 0);
}

tree::TerminalNode* WhooshParser::TermContext::TOKEN() {
  return getToken(WhooshParser::TOKEN, 0);
}


size_t WhooshParser::TermContext::getRuleIndex() const {
  return WhooshParser::RuleTerm;
}

void WhooshParser::TermContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<WhooshListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterTerm(this);
}

void WhooshParser::TermContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<WhooshListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitTerm(this);
}


std::any WhooshParser::TermContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<WhooshVisitor*>(visitor))
    return parserVisitor->visitTerm(this);
  else
    return visitor->visitChildren(this);
}

WhooshParser::TermContext* WhooshParser::term() {
  TermContext *_localctx = _tracker.createInstance<TermContext>(_ctx, getState());
  enterRule(_localctx, 0, WhooshParser::RuleTerm);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(4);
    match(WhooshParser::CATEGORY);
    setState(5);
    match(WhooshParser::T__0);
    setState(6);
    match(WhooshParser::TOKEN);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ExprContext ------------------------------------------------------------------

WhooshParser::ExprContext::ExprContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t WhooshParser::ExprContext::getRuleIndex() const {
  return WhooshParser::RuleExpr;
}

void WhooshParser::ExprContext::copyFrom(ExprContext *ctx) {
  ParserRuleContext::copyFrom(ctx);
}

//----------------- WrappedExprContext ------------------------------------------------------------------

WhooshParser::ExprContext* WhooshParser::WrappedExprContext::expr() {
  return getRuleContext<WhooshParser::ExprContext>(0);
}

WhooshParser::WrappedExprContext::WrappedExprContext(ExprContext *ctx) { copyFrom(ctx); }

void WhooshParser::WrappedExprContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<WhooshListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterWrappedExpr(this);
}
void WhooshParser::WrappedExprContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<WhooshListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitWrappedExpr(this);
}

std::any WhooshParser::WrappedExprContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<WhooshVisitor*>(visitor))
    return parserVisitor->visitWrappedExpr(this);
  else
    return visitor->visitChildren(this);
}
//----------------- NotExprContext ------------------------------------------------------------------

WhooshParser::ExprContext* WhooshParser::NotExprContext::expr() {
  return getRuleContext<WhooshParser::ExprContext>(0);
}

WhooshParser::NotExprContext::NotExprContext(ExprContext *ctx) { copyFrom(ctx); }

void WhooshParser::NotExprContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<WhooshListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterNotExpr(this);
}
void WhooshParser::NotExprContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<WhooshListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitNotExpr(this);
}

std::any WhooshParser::NotExprContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<WhooshVisitor*>(visitor))
    return parserVisitor->visitNotExpr(this);
  else
    return visitor->visitChildren(this);
}
//----------------- TermExprContext ------------------------------------------------------------------

WhooshParser::TermContext* WhooshParser::TermExprContext::term() {
  return getRuleContext<WhooshParser::TermContext>(0);
}

WhooshParser::TermExprContext::TermExprContext(ExprContext *ctx) { copyFrom(ctx); }

void WhooshParser::TermExprContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<WhooshListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterTermExpr(this);
}
void WhooshParser::TermExprContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<WhooshListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitTermExpr(this);
}

std::any WhooshParser::TermExprContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<WhooshVisitor*>(visitor))
    return parserVisitor->visitTermExpr(this);
  else
    return visitor->visitChildren(this);
}
//----------------- OrExprContext ------------------------------------------------------------------

std::vector<WhooshParser::ExprContext *> WhooshParser::OrExprContext::expr() {
  return getRuleContexts<WhooshParser::ExprContext>();
}

WhooshParser::ExprContext* WhooshParser::OrExprContext::expr(size_t i) {
  return getRuleContext<WhooshParser::ExprContext>(i);
}

WhooshParser::OrExprContext::OrExprContext(ExprContext *ctx) { copyFrom(ctx); }

void WhooshParser::OrExprContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<WhooshListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterOrExpr(this);
}
void WhooshParser::OrExprContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<WhooshListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitOrExpr(this);
}

std::any WhooshParser::OrExprContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<WhooshVisitor*>(visitor))
    return parserVisitor->visitOrExpr(this);
  else
    return visitor->visitChildren(this);
}
//----------------- XorExprContext ------------------------------------------------------------------

std::vector<WhooshParser::ExprContext *> WhooshParser::XorExprContext::expr() {
  return getRuleContexts<WhooshParser::ExprContext>();
}

WhooshParser::ExprContext* WhooshParser::XorExprContext::expr(size_t i) {
  return getRuleContext<WhooshParser::ExprContext>(i);
}

WhooshParser::XorExprContext::XorExprContext(ExprContext *ctx) { copyFrom(ctx); }

void WhooshParser::XorExprContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<WhooshListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterXorExpr(this);
}
void WhooshParser::XorExprContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<WhooshListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitXorExpr(this);
}

std::any WhooshParser::XorExprContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<WhooshVisitor*>(visitor))
    return parserVisitor->visitXorExpr(this);
  else
    return visitor->visitChildren(this);
}
//----------------- AndExprContext ------------------------------------------------------------------

std::vector<WhooshParser::ExprContext *> WhooshParser::AndExprContext::expr() {
  return getRuleContexts<WhooshParser::ExprContext>();
}

WhooshParser::ExprContext* WhooshParser::AndExprContext::expr(size_t i) {
  return getRuleContext<WhooshParser::ExprContext>(i);
}

WhooshParser::AndExprContext::AndExprContext(ExprContext *ctx) { copyFrom(ctx); }

void WhooshParser::AndExprContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<WhooshListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterAndExpr(this);
}
void WhooshParser::AndExprContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<WhooshListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitAndExpr(this);
}

std::any WhooshParser::AndExprContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<WhooshVisitor*>(visitor))
    return parserVisitor->visitAndExpr(this);
  else
    return visitor->visitChildren(this);
}

WhooshParser::ExprContext* WhooshParser::expr() {
   return expr(0);
}

WhooshParser::ExprContext* WhooshParser::expr(int precedence) {
  ParserRuleContext *parentContext = _ctx;
  size_t parentState = getState();
  WhooshParser::ExprContext *_localctx = _tracker.createInstance<ExprContext>(_ctx, parentState);
  WhooshParser::ExprContext *previousContext = _localctx;
  (void)previousContext; // Silence compiler, in case the context is not used by generated code.
  size_t startState = 2;
  enterRecursionRule(_localctx, 2, WhooshParser::RuleExpr, precedence);

    

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    unrollRecursionContexts(parentContext);
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(16);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case WhooshParser::CATEGORY: {
        _localctx = _tracker.createInstance<TermExprContext>(_localctx);
        _ctx = _localctx;
        previousContext = _localctx;

        setState(9);
        term();
        break;
      }

      case WhooshParser::T__1: {
        _localctx = _tracker.createInstance<WrappedExprContext>(_localctx);
        _ctx = _localctx;
        previousContext = _localctx;
        setState(10);
        match(WhooshParser::T__1);
        setState(11);
        expr(0);
        setState(12);
        match(WhooshParser::T__2);
        break;
      }

      case WhooshParser::T__6: {
        _localctx = _tracker.createInstance<NotExprContext>(_localctx);
        _ctx = _localctx;
        previousContext = _localctx;
        setState(14);
        match(WhooshParser::T__6);
        setState(15);
        antlrcpp::downCast<NotExprContext *>(_localctx)->right = expr(2);
        break;
      }

    default:
      throw NoViableAltException(this);
    }
    _ctx->stop = _input->LT(-1);
    setState(31);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 2, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        setState(29);
        _errHandler->sync(this);
        switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 1, _ctx)) {
        case 1: {
          auto newContext = _tracker.createInstance<OrExprContext>(_tracker.createInstance<ExprContext>(parentContext, parentState));
          _localctx = newContext;
          newContext->left = previousContext;
          pushNewRecursionContext(newContext, startState, RuleExpr);
          setState(18);

          if (!(precpred(_ctx, 5))) throw FailedPredicateException(this, "precpred(_ctx, 5)");
          setState(19);
          match(WhooshParser::T__3);
          setState(20);
          antlrcpp::downCast<OrExprContext *>(_localctx)->right = expr(6);
          break;
        }

        case 2: {
          auto newContext = _tracker.createInstance<AndExprContext>(_tracker.createInstance<ExprContext>(parentContext, parentState));
          _localctx = newContext;
          newContext->left = previousContext;
          pushNewRecursionContext(newContext, startState, RuleExpr);
          setState(21);

          if (!(precpred(_ctx, 4))) throw FailedPredicateException(this, "precpred(_ctx, 4)");
          setState(22);
          match(WhooshParser::T__4);
          setState(23);
          antlrcpp::downCast<AndExprContext *>(_localctx)->right = expr(5);
          break;
        }

        case 3: {
          auto newContext = _tracker.createInstance<XorExprContext>(_tracker.createInstance<ExprContext>(parentContext, parentState));
          _localctx = newContext;
          newContext->left = previousContext;
          pushNewRecursionContext(newContext, startState, RuleExpr);
          setState(24);

          if (!(precpred(_ctx, 3))) throw FailedPredicateException(this, "precpred(_ctx, 3)");
          setState(25);
          match(WhooshParser::T__5);
          setState(26);
          antlrcpp::downCast<XorExprContext *>(_localctx)->right = expr(4);
          break;
        }

        case 4: {
          auto newContext = _tracker.createInstance<AndExprContext>(_tracker.createInstance<ExprContext>(parentContext, parentState));
          _localctx = newContext;
          newContext->left = previousContext;
          pushNewRecursionContext(newContext, startState, RuleExpr);
          setState(27);

          if (!(precpred(_ctx, 1))) throw FailedPredicateException(this, "precpred(_ctx, 1)");
          setState(28);
          antlrcpp::downCast<AndExprContext *>(_localctx)->right = expr(2);
          break;
        }

        default:
          break;
        } 
      }
      setState(33);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 2, _ctx);
    }
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }
  return _localctx;
}

bool WhooshParser::sempred(RuleContext *context, size_t ruleIndex, size_t predicateIndex) {
  switch (ruleIndex) {
    case 1: return exprSempred(antlrcpp::downCast<ExprContext *>(context), predicateIndex);

  default:
    break;
  }
  return true;
}

bool WhooshParser::exprSempred(ExprContext *_localctx, size_t predicateIndex) {
  switch (predicateIndex) {
    case 0: return precpred(_ctx, 5);
    case 1: return precpred(_ctx, 4);
    case 2: return precpred(_ctx, 3);
    case 3: return precpred(_ctx, 1);

  default:
    break;
  }
  return true;
}

void WhooshParser::initialize() {
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  whooshParserInitialize();
#else
  ::antlr4::internal::call_once(whooshParserOnceFlag, whooshParserInitialize);
#endif
}

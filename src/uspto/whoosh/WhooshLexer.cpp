
// Generated from src/uspto/whoosh/Whoosh.g4 by ANTLR 4.13.1


#include "WhooshLexer.h"


using namespace antlr4;

using namespace whoosh;


using namespace antlr4;

namespace {

struct WhooshLexerStaticData final {
  WhooshLexerStaticData(std::vector<std::string> ruleNames,
                          std::vector<std::string> channelNames,
                          std::vector<std::string> modeNames,
                          std::vector<std::string> literalNames,
                          std::vector<std::string> symbolicNames)
      : ruleNames(std::move(ruleNames)), channelNames(std::move(channelNames)),
        modeNames(std::move(modeNames)), literalNames(std::move(literalNames)),
        symbolicNames(std::move(symbolicNames)),
        vocabulary(this->literalNames, this->symbolicNames) {}

  WhooshLexerStaticData(const WhooshLexerStaticData&) = delete;
  WhooshLexerStaticData(WhooshLexerStaticData&&) = delete;
  WhooshLexerStaticData& operator=(const WhooshLexerStaticData&) = delete;
  WhooshLexerStaticData& operator=(WhooshLexerStaticData&&) = delete;

  std::vector<antlr4::dfa::DFA> decisionToDFA;
  antlr4::atn::PredictionContextCache sharedContextCache;
  const std::vector<std::string> ruleNames;
  const std::vector<std::string> channelNames;
  const std::vector<std::string> modeNames;
  const std::vector<std::string> literalNames;
  const std::vector<std::string> symbolicNames;
  const antlr4::dfa::Vocabulary vocabulary;
  antlr4::atn::SerializedATNView serializedATN;
  std::unique_ptr<antlr4::atn::ATN> atn;
};

::antlr4::internal::OnceFlag whooshlexerLexerOnceFlag;
#if ANTLR4_USE_THREAD_LOCAL_CACHE
static thread_local
#endif
WhooshLexerStaticData *whooshlexerLexerStaticData = nullptr;

void whooshlexerLexerInitialize() {
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  if (whooshlexerLexerStaticData != nullptr) {
    return;
  }
#else
  assert(whooshlexerLexerStaticData == nullptr);
#endif
  auto staticData = std::make_unique<WhooshLexerStaticData>(
    std::vector<std::string>{
      "T__0", "T__1", "T__2", "T__3", "T__4", "T__5", "T__6", "WS", "TOKEN"
    },
    std::vector<std::string>{
      "DEFAULT_TOKEN_CHANNEL", "HIDDEN"
    },
    std::vector<std::string>{
      "DEFAULT_MODE"
    },
    std::vector<std::string>{
      "", "':'", "'('", "')'", "'OR'", "'AND'", "'XOR'", "'NOT'"
    },
    std::vector<std::string>{
      "", "", "", "", "", "", "", "", "WS", "TOKEN"
    }
  );
  static const int32_t serializedATNSegment[] = {
  	4,0,9,52,6,-1,2,0,7,0,2,1,7,1,2,2,7,2,2,3,7,3,2,4,7,4,2,5,7,5,2,6,7,6,
  	2,7,7,7,2,8,7,8,1,0,1,0,1,1,1,1,1,2,1,2,1,3,1,3,1,3,1,4,1,4,1,4,1,4,1,
  	5,1,5,1,5,1,5,1,6,1,6,1,6,1,6,1,7,4,7,42,8,7,11,7,12,7,43,1,7,1,7,1,8,
  	4,8,49,8,8,11,8,12,8,50,0,0,9,1,1,3,2,5,3,7,4,9,5,11,6,13,7,15,8,17,9,
  	1,0,2,1,0,32,32,4,0,46,57,65,90,95,95,97,122,53,0,1,1,0,0,0,0,3,1,0,0,
  	0,0,5,1,0,0,0,0,7,1,0,0,0,0,9,1,0,0,0,0,11,1,0,0,0,0,13,1,0,0,0,0,15,
  	1,0,0,0,0,17,1,0,0,0,1,19,1,0,0,0,3,21,1,0,0,0,5,23,1,0,0,0,7,25,1,0,
  	0,0,9,28,1,0,0,0,11,32,1,0,0,0,13,36,1,0,0,0,15,41,1,0,0,0,17,48,1,0,
  	0,0,19,20,5,58,0,0,20,2,1,0,0,0,21,22,5,40,0,0,22,4,1,0,0,0,23,24,5,41,
  	0,0,24,6,1,0,0,0,25,26,5,79,0,0,26,27,5,82,0,0,27,8,1,0,0,0,28,29,5,65,
  	0,0,29,30,5,78,0,0,30,31,5,68,0,0,31,10,1,0,0,0,32,33,5,88,0,0,33,34,
  	5,79,0,0,34,35,5,82,0,0,35,12,1,0,0,0,36,37,5,78,0,0,37,38,5,79,0,0,38,
  	39,5,84,0,0,39,14,1,0,0,0,40,42,7,0,0,0,41,40,1,0,0,0,42,43,1,0,0,0,43,
  	41,1,0,0,0,43,44,1,0,0,0,44,45,1,0,0,0,45,46,6,7,0,0,46,16,1,0,0,0,47,
  	49,7,1,0,0,48,47,1,0,0,0,49,50,1,0,0,0,50,48,1,0,0,0,50,51,1,0,0,0,51,
  	18,1,0,0,0,3,0,43,50,1,6,0,0
  };
  staticData->serializedATN = antlr4::atn::SerializedATNView(serializedATNSegment, sizeof(serializedATNSegment) / sizeof(serializedATNSegment[0]));

  antlr4::atn::ATNDeserializer deserializer;
  staticData->atn = deserializer.deserialize(staticData->serializedATN);

  const size_t count = staticData->atn->getNumberOfDecisions();
  staticData->decisionToDFA.reserve(count);
  for (size_t i = 0; i < count; i++) { 
    staticData->decisionToDFA.emplace_back(staticData->atn->getDecisionState(i), i);
  }
  whooshlexerLexerStaticData = staticData.release();
}

}

WhooshLexer::WhooshLexer(CharStream *input) : Lexer(input) {
  WhooshLexer::initialize();
  _interpreter = new atn::LexerATNSimulator(this, *whooshlexerLexerStaticData->atn, whooshlexerLexerStaticData->decisionToDFA, whooshlexerLexerStaticData->sharedContextCache);
}

WhooshLexer::~WhooshLexer() {
  delete _interpreter;
}

std::string WhooshLexer::getGrammarFileName() const {
  return "Whoosh.g4";
}

const std::vector<std::string>& WhooshLexer::getRuleNames() const {
  return whooshlexerLexerStaticData->ruleNames;
}

const std::vector<std::string>& WhooshLexer::getChannelNames() const {
  return whooshlexerLexerStaticData->channelNames;
}

const std::vector<std::string>& WhooshLexer::getModeNames() const {
  return whooshlexerLexerStaticData->modeNames;
}

const dfa::Vocabulary& WhooshLexer::getVocabulary() const {
  return whooshlexerLexerStaticData->vocabulary;
}

antlr4::atn::SerializedATNView WhooshLexer::getSerializedATN() const {
  return whooshlexerLexerStaticData->serializedATN;
}

const atn::ATN& WhooshLexer::getATN() const {
  return *whooshlexerLexerStaticData->atn;
}




void WhooshLexer::initialize() {
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  whooshlexerLexerInitialize();
#else
  ::antlr4::internal::call_once(whooshlexerLexerOnceFlag, whooshlexerLexerInitialize);
#endif
}

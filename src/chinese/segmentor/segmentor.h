/****************************************************************
 *                                                              *
 * segmentor.h - the sentence segmentor.                        *
 *                                                              *
 * Author: Yue Zhang                                            *
 *                                                              *
 * Computing Laboratory, Oxford. 2006.10                        *
 *                                                              *
 ****************************************************************/

#ifndef CHINESE_SEGMENTOR_H
#define CHINESE_SEGMENTOR_H 1

#include "base_include.h"

/*===============================================================
 *
 * Global items
 *
 *==============================================================*/

namespace chinese {

class CSegmentor;

//typedef CHashMap<CWord, int> CWordIntMap ; 
namespace segmentor {

#include "feature.h"

} // namespace segmentor

/*===============================================================
 *
 * CSegmentor - the segmentor for Chinese 
 *
 * Segment a sentence character by character, generating possible
 * output sentences into a beam agenda. 
 *
 * Given an input sentence, m_lWordCache caches the string words
 * in certain positions. 
 * For example: suppose that a sentence is
 *              A B C D E F
 *              m_lWordCache[0][0] = A
 *              m_lWordCache[0][4] = A B C D E
 *              m_lWordCache[1][5] = B C D E F
 * Note: there are space wastes, but this seem to be the most time
 *       efficient. 
 *
 *==============================================================*/

class CSegmentor : public segmentor::CSegmentorImpl {

private:
   segmentor::CFeatureHandle *m_Feature;
   CWord *m_lWordCache;
   CWordDictionary *m_CharCat;
   CLexiconSet *m_WordLst;
   CCharCatDictionary *m_CharCatForRules; // use rules to segment foreign words?
   bool m_bTrain;

//-------------------------------------------------------------
// Constructor destructor
public:
   CSegmentor(const string &sFeatureDBPath, bool bTrain=false, const string &sCharCatFile="", const string &sLexiconFile="") : segmentor::CSegmentorImpl(), m_bTrain(bTrain), m_CharCat(0), m_WordLst(0), m_CharCatForRules(0) { 
      m_Feature = new segmentor::CFeatureHandle(this, sFeatureDBPath, bTrain); 
      m_lWordCache = new CWord[segmentor::MAX_SENTENCE_SIZE*segmentor::MAX_WORD_SIZE];
      if (!bTrain) {
         if (m_Feature->m_bCharCat && sCharCatFile.empty())
            THROW("model requires character category knowledge but this file is not specified");
         if (m_Feature->m_bLexicon && sLexiconFile.empty())
            THROW("model requires lexicon knowledge but this file is not specified");
      }
      if (!sCharCatFile.empty()) loadCharCat(sCharCatFile);
      if (!sLexiconFile.empty()) loadLexiconDict(sLexiconFile);
   }
   virtual ~CSegmentor() { delete m_Feature; delete [] m_lWordCache; clearKnowledge(); }
   CSegmentor(CSegmentor& segmentor) { throw("CSegmentor does not support copy constructor!"); }

//-------------------------------------------------------------
// Main interface
public:
   void train(const CSentenceRaw* sentence, const CSentenceRaw *correct, int &round);
   void segment(const CSentenceRaw* sentence, CSentenceRaw *retval, double *out_scores=NULL, int nBest=1);

   bool isTraining() const { return m_bTrain; }

#ifdef NO_NEG_FEATURE

   void extractPosFeatures( const CSentenceRaw *sent ) {
      this->m_Feature->extractPosFeatures(sent);
   }

#endif

   void updateScores(const CSentenceRaw* output, const CSentenceRaw* correct, int round) {
      m_Feature->updateScoreVector(output, correct, round);
   }
   void finishTraining(int round) {
      m_Feature->computeAverageFeatureWeights(round);
      m_Feature->saveScores();
   }

   segmentor::CWeight &getWeights() { return m_Feature->getWeights(); }

//-------------------------------------------------------------
// Knowledge related 
public:
   void loadCharCat(const string &sFile) {
      if (m_CharCat==0) {
         m_CharCat = new CWordDictionary(2719);
         m_CharCat->load(sFile);
      }
   }
   void loadLexiconDict(const string &sFile) {
      if (m_WordLst==0)
         m_WordLst = new CLexiconSet;
      ifstream is(sFile.c_str());
      assert(is.is_open());
      is >> (*m_WordLst);
      is.close();
   }
   void useRules(bool bUseRules) {
      if ( bUseRules && m_CharCatForRules==0 ) {
         m_CharCatForRules = new CCharCatDictionary;
      }
      if ( !bUseRules && m_CharCatForRules ) {
         delete m_CharCatForRules;
         m_CharCatForRules = 0;
      }
   }
   void clearKnowledge() {
      if (m_CharCat) { delete m_CharCat; m_CharCat = 0; }
      if (m_WordLst) { delete m_WordLst; m_WordLst = 0; }
      if (m_CharCatForRules) { delete m_CharCatForRules; m_CharCatForRules = 0; }
   }

   bool wordInLexicon(const CWord &word) {
      if (m_WordLst==0)
         return false;
      return m_WordLst->find(word, 0); // non-zero value means member
   }
   bool hasCharTypeKnowledge() {
      return m_CharCat != 0;
   }
   unsigned charType(const CWord &ch) {
      return m_CharCat->lookup(ch);
   }

//-------------------------------------------------------------
// Word cache
public:
   const CWord& findWordFromCache(const int &start, const int &length, const CSentenceRaw* sentence) {
      if (m_lWordCache[ start * segmentor::MAX_WORD_SIZE + length - 1 ].empty()) { // empty string
         static string temp; 
         static unsigned int i; 
         temp.clear();
         for ( i = start; i < start+length ; i++ ) 
            temp += sentence->at(i) ; 
         m_lWordCache[ start * segmentor::MAX_WORD_SIZE + length - 1 ].setString(temp);
      }
      return m_lWordCache[ start * segmentor::MAX_WORD_SIZE + length - 1 ];
   }
   const CWord& replaceWordToCache(const int &start, const int &length, const CSentenceRaw* sentence) {
      if (m_lWordCache[ start * segmentor::MAX_WORD_SIZE + length - 1 ].empty()||
          m_lWordCache[ start * segmentor::MAX_WORD_SIZE + length - 1 ].unknown()) { // empty string
         static string temp; 
         static unsigned int i; 
         temp.clear();
         for ( i = start; i < start+length ; i++ ) 
            temp += sentence->at(i) ; 
         m_lWordCache[ start * segmentor::MAX_WORD_SIZE + length - 1 ] = (temp);
      }
      return m_lWordCache[ start * segmentor::MAX_WORD_SIZE + length - 1 ];
   }
   void clearWordCache() {
      for (int i=0; i<segmentor::MAX_SENTENCE_SIZE; i++)
         for (int j=0; j<segmentor::MAX_WORD_SIZE; j++) 
            m_lWordCache[i*segmentor::MAX_WORD_SIZE+j].clear();
   }
};

} // namespace chinese

#endif



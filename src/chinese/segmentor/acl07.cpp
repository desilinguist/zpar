// Copyright (C) University of Oxford 2010
/****************************************************************
 *                                                              *
 * segmentor.cpp - the chinese segmentor                        *
 *                                                              *
 * This implementation uses beam search ageanda, see chart.cpp  *
 *                                                              *
 * Author: Yue Zhang                                            *
 *                                                              *
 * Computing Laboratory, Oxford. 2006.10                        *
 *                                                              *
 ****************************************************************/

#include "segmentor.h"

using namespace chinese;
using namespace chinese::segmentor;

CWord g_emptyWord("");

/*===============================================================
 *
 * CFeatureHandle - handles the features for the segmentor
 *
 *==============================================================*/

/*---------------------------------------------------------------
 *
 * getGlobalScore - get the global score for sentence
 *
 *--------------------------------------------------------------*/

int CFeatureHandle::getGlobalScore(const CStringVector* sentence, const CStateItem* item){
   int nReturn = 0;
   for (int i=0; i<item->m_nLength; ++i)
      nReturn += getLocalScore(sentence, item, i);
   return nReturn;
}

/*---------------------------------------------------------------
 *
 * getLocalScore - get the local score for a word in sentence
 *
 * When bigram is needed from the beginning of sentence, the empty word are used. 
 *
 * This implies that empty words should not be used in other 
 * situations. 
 *
 *--------------------------------------------------------------*/

int CFeatureHandle::getLocalScore(const CStringVector* sentence, const CStateItem* item, int index){
   static int nReturn; 
   static int score_index;
   static int tmp_i, tmp_j;
   static int length, last_length, word_length;
   static int start, end, last_start, last_end;
   score_index = m_bTrain ? CScore<SCORE_TYPE>::eNonAverage : CScore<SCORE_TYPE>::eAverage ;
   start = item->getWordStart(index);
   end = item->getWordEnd(index);
   length = item->getWordLength(index); 
   last_start = index>0 ? item->getWordStart(index-1) : 0; // make sure that this is only used when index>0
   last_end = index>0 ? item->getWordEnd(index-1) : 0; // make sure that this is only used when index>0
   last_length = index>0 ? item->getWordLength(index-1) : 0;  // similar to the above
   word_length = length ; 
   // abstd::cout the words
   const CWord &word=m_parent->findWordFromCache(start, length, sentence); 
   const CWord &last_word = index>0 ? m_parent->findWordFromCache(last_start, last_length, sentence) : g_emptyWord; // use empty word for sentence beginners. 
   static CTwoWords two_word;
   two_word.refer(&word, &last_word);
   // abstd::cout the chars
   const CWord &first_char=m_parent->findWordFromCache(start, 1, sentence);
   const CWord &last_char=m_parent->findWordFromCache(end, 1, sentence);
   const CWord &first_char_last_word = index>0 ? m_parent->findWordFromCache(last_start, 1, sentence) : g_emptyWord;
   const CWord &last_char_last_word = index>0 ? m_parent->findWordFromCache(last_end, 1, sentence) : g_emptyWord;
   static CTwoWords first_and_last_char, lastword_firstchar, currentword_lastchar, firstcharlastword_word, lastword_lastchar;
   first_and_last_char.refer(&first_char, &last_char);
   const CWord &two_char = index>0 ? m_parent->findWordFromCache(last_end, 2, sentence) : g_emptyWord;
   if (index>0) {
      lastword_firstchar.refer(&last_word, &first_char);
      currentword_lastchar.refer(&word, &last_char_last_word);
      firstcharlastword_word.refer(&first_char_last_word, &first_char);
      lastword_lastchar.refer(&last_char_last_word, &last_char);
   }
   // abstd::cout the length
   if(length>LENGTH_MAX-1)length=LENGTH_MAX-1;
   if(last_length>LENGTH_MAX-1)last_length=LENGTH_MAX-1;
   //
   // adding scores with features
   //
   nReturn = m_weights.m_mapSeenWords.getScore(word, score_index); 
   nReturn += m_weights.m_mapLastWordByWord.getScore(two_word, score_index);
   if (length==1) 
      nReturn += m_weights.m_mapOneCharWord.getScore(word, score_index);
   else {
      nReturn += m_weights.m_mapFirstAndLastChars.getScore(first_and_last_char, score_index);
      for (int j=0; j<word_length-1; j++) 
         nReturn += m_weights.m_mapConsecutiveChars.getScore(m_parent->findWordFromCache(start+j, 2, sentence), score_index);

      nReturn += m_weights.m_mapLengthByFirstChar.getScore(std::make_pair(first_char, length), score_index);
      nReturn += m_weights.m_mapLengthByLastChar.getScore(std::make_pair(last_char, length), score_index);
   }
   if (index>0) {
      nReturn += m_weights.m_mapSeparateChars.getScore(two_char, score_index); 
      
      nReturn += m_weights.m_mapLastWordFirstChar.getScore(lastword_firstchar, score_index);
      nReturn += m_weights.m_mapCurrentWordLastChar.getScore(currentword_lastchar, score_index);
      nReturn += m_weights.m_mapFirstCharLastWordByWord.getScore(firstcharlastword_word, score_index);
      nReturn += m_weights.m_mapLastWordByLastChar.getScore(lastword_lastchar, score_index);
      
      nReturn += m_weights.m_mapLengthByLastWord.getScore(std::make_pair(last_word, length), score_index);
      nReturn += m_weights.m_mapLastLengthByWord.getScore(std::make_pair(word, last_length), score_index);
   }

   return nReturn;
}

/*---------------------------------------------------------------
 *
 * updateScoreVector - update the score std::vector by input
 *                     this is used in training to adjust params
 *
 * Inputs: the outout and the correct examples
 *
 * Affects: m_bScoreModified, which leads to saveScores on destructor
 *
 *--------------------------------------------------------------*/

void CFeatureHandle::updateScoreVector(const CStringVector* outout, const CStringVector* correct, int round) {
   if ( *outout == *correct ) return;
   for (int i=0; i<outout->size(); ++i)
      updateLocalFeatureVector(eSubtract, outout, i, round);
   for (int j=0; j<correct->size(); ++j)
      updateLocalFeatureVector(eAdd, correct, j, round);
   m_bScoreModified = true;
}

/*---------------------------------------------------------------
 *
 * updateLocalFeatureVector - update the given feature vector with
 *                            the local feature vector for a given
 *                            sentence. This is a private member only 
 *                            used by updateGlobalFeatureVector and is
 *                            only used for training. 
 *
 *--------------------------------------------------------------*/

void CFeatureHandle::updateLocalFeatureVector(SCORE_UPDATE method, const CStringVector* outout, int index, int round) { 
   // abstd::cout words              
   CWord word = outout->at(index);
   CWord last_word = index>0 ? outout->at(index-1) : g_emptyWord;
   CTwoWords two_word;
   two_word.allocate(word.str(), last_word.str());
   CStringVector chars;
   chars.clear(); getCharactersFromUTF8String(word.str(), &chars);
   // abstd::cout length
   int length = getUTF8StringLength(word.str()); if (length > LENGTH_MAX-1) length = LENGTH_MAX-1;
   int last_length = getUTF8StringLength(last_word.str()); if (last_length > LENGTH_MAX-1) last_length = LENGTH_MAX-1;
   // abstd::cout chars  
   CWord first_char = getFirstCharFromUTF8String(word.str());
   CWord last_char = getLastCharFromUTF8String(word.str());
   CWord first_char_last_word = index>0 ? getFirstCharFromUTF8String(last_word.str()) : g_emptyWord;
   CWord last_char_last_word = index>0 ? getLastCharFromUTF8String(last_word.str()) : g_emptyWord;
   CWord two_char = index>0 ? last_char_last_word.str() + first_char.str() : g_emptyWord;
   CTwoWords first_and_last_char, lastword_firstchar, currentword_lastchar, firstcharlastword_word, lastword_lastchar;
   first_and_last_char.allocate(first_char.str(), last_char.str());
   if (index>0) {
      lastword_firstchar.allocate(last_word.str(), first_char.str());
      currentword_lastchar.allocate(word.str(), last_char_last_word.str());
      firstcharlastword_word.allocate(first_char_last_word.str(), first_char.str());
      lastword_lastchar.allocate(last_char_last_word.str(), last_char.str());
   }
   
   SCORE_TYPE amount = ( (method==eAdd) ? 1 : -1 ) ;

   m_weights.m_mapSeenWords.updateScore(word, amount, round);
   m_weights.m_mapLastWordByWord.updateScore(two_word, amount, round);
   if (length==1) m_weights.m_mapOneCharWord.updateScore(first_char, amount, round);
   else {
      m_weights.m_mapFirstAndLastChars.updateScore(first_and_last_char, amount, round);
      for (int j=0; j<chars.size()-1; j++) {
         m_weights.m_mapConsecutiveChars.updateScore(chars[j]+chars[j+1], amount, round);
      }
      m_weights.m_mapLengthByFirstChar.updateScore(std::make_pair(first_char, length), amount, round);
      m_weights.m_mapLengthByLastChar.updateScore(std::make_pair(last_char, length), amount, round);
   } 
   if (index>0) {
      m_weights.m_mapSeparateChars.updateScore(two_char, amount, round);
      
      m_weights.m_mapLastWordFirstChar.updateScore(lastword_firstchar, amount, round);
      m_weights.m_mapCurrentWordLastChar.updateScore(currentword_lastchar, amount, round);
      
      m_weights.m_mapFirstCharLastWordByWord.updateScore(firstcharlastword_word, amount, round);
      m_weights.m_mapLastWordByLastChar.updateScore(lastword_lastchar, amount, round);

      m_weights.m_mapLengthByLastWord.updateScore(std::make_pair(last_word, length), amount, round);
      m_weights.m_mapLastLengthByWord.updateScore(std::make_pair(word, last_length), amount, round);
   }
}

/*===============================================================
 *
 * CSegmentor - the segmentor for Chinese 
 *
 *==============================================================*/

/*---------------------------------------------------------------
 *
 * train - do automatic training process
 * [NOT USED]
 *
 *--------------------------------------------------------------*/

void CSegmentor::train(const CStringVector* sentence, const CStringVector* correct, int & round) {
   assert(1==0); // train by the configurable training algorithm
}

/*---------------------------------------------------------------
 *
 * segment - do word segmentation on  a sentence
 *
 *--------------------------------------------------------------*/

void CSegmentor::segment(const CStringVector* sentence_input, CStringVector *vReturn, double *out_scores, int nBest) {
   clock_t total_start_time = clock();;
   CStateItem *pGenerator, *pCandidate;
   unsigned index;                              // the index of the current char
   unsigned j, k;                               // temporary index
   int subtract_score;                          // the score to be subtracted (previous item)
   static unsigned doneLastWord[MAX_SENTENCE_SIZE];

   static CStringVector sentence;
   static CRule rules(m_Feature->m_bRule);
   rules.segment(sentence_input, &sentence);
   const unsigned length = sentence.size();

   if (length > MAX_SENTENCE_SIZE) {
      std::cerr << "The size of the sentence is " << length << " characters, which is larger than the limit of the system (" << MAX_SENTENCE_SIZE <<std::endl;
      vReturn->clear();
      return;
   }
   assert(vReturn!=NULL);

   //clock_t start_time = clock();
   TRACE("Initialising the segmentation process...");
   vReturn->clear();
   clearWordCache(); 
   m_Agenda->clear();
   pCandidate = m_Agenda->candidateItem();      // make the first item
   pCandidate->clear();                         // restore state using clean
   m_Agenda->pushCandidate();                   // and push it back
   m_Agenda->nextRound();                       // as the generator item
   if (nBest == 1)                              // optimization for one best
      for (j=0; j<MAX_SENTENCE_SIZE; ++j) doneLastWord[j] = 0;

   TRACE("Segmenting started");
   //TRACE("initialisation time: " << clock() - start_time);
   for (index=0; index<length; index++) {
      // generate new state itmes for each character
      pGenerator = m_Agenda->generatorStart();
      for (j=0; j<m_Agenda->generatorSize(); ++j) {
         // 1. generate new items according to each previous item. 
         if (pGenerator->m_nLength>0) k = pGenerator->getWordStart(pGenerator->m_nLength-1);
         // If we only ask 1-best, then we take only the best among those with the last word
         if ( ( nBest > 1 || pGenerator->m_nLength==0 || doneLastWord[k]<index+1 ) && 
              rules.canSeparate( index ) 
            ) {  
            pCandidate = m_Agenda->candidateItem();
            pCandidate->copy(pGenerator);
            pCandidate->append(index);
            pCandidate->m_nScore += m_Feature->getLocalScore(&sentence, pCandidate, pCandidate->m_nLength-1); 
            m_Agenda->pushCandidate();
            if (nBest == 1 && pGenerator->m_nLength>0) doneLastWord[k] = index+1;
         }
         // 2. generate by replacing items
         if ( index > 0 && rules.canAppend(index) ) {
            pCandidate = m_Agenda->candidateItem();
            pCandidate->copy(pGenerator);
            subtract_score = m_Feature->getLocalScore(&sentence, pGenerator, pGenerator->m_nLength-1);
            pCandidate->m_nScore -= subtract_score;
            pCandidate->replace(index);
            pCandidate->m_nScore += m_Feature->getLocalScore(&sentence, pCandidate, pCandidate->m_nLength-1);
            m_Agenda->pushCandidate();
         }
         pGenerator = m_Agenda->generatorNext();  // next generator
      }
      m_Agenda->nextRound(); // move round
   }
   // now generate outout sentence
   // n-best list will be stored in array
   // from the addr vReturn
   TRACE("Outputing sentence");
   for (k=0; k<nBest; ++k) {
      // clear
      vReturn[k].clear();
      if (out_scores!=NULL) 
         out_scores[k] = 0;
      // assign retval
      if (k<m_Agenda->generatorSize()) {
         pGenerator = m_Agenda->generator(k);
         for (j=0; j<pGenerator->m_nLength; j++) {
            std::string temp = "";
            for (unsigned l = pGenerator->getWordStart(j); l <= pGenerator->getWordEnd(j); ++l) {
               assert(sentence.at(l)!=" "); // [SPACE]
               temp += sentence.at(l);
            }
            vReturn[k].push_back(temp);
         }
         if (out_scores!=NULL)
            out_scores[k] = pGenerator->m_nScore;
      }
   }
   TRACE("Done, the best score: " << pGenerator->m_nScore);
   TRACE("total time spent: " << double(clock() - total_start_time)/CLOCKS_PER_SEC);
}


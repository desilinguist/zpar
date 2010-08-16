#ifndef _GENERAL_CONPARSER_MACROS_H
#define _GENERAL_CONPARSER_MACROS_H

// early update? 
#define EARLY_UPDATE 1

// The size of agenda
const unsigned long AGENDA_SIZE = 16;

// The size of a sentence and the words
const int MAX_SENTENCE_SIZE = 256 ; 
const int MAX_SENTENCE_SIZE_BITS = 8 ; 

inline unsigned long normalize3(const unsigned long &x) {
   assert(x!=0); return x>2 ? 2 : x==2 ? 1 : 0;
}

inline unsigned long normalize16(const unsigned long &x) {
   return x>15 ? 15 : x;
}

inline unsigned long normalize510(const unsigned long &x) {
   return x>10 ? 6 : x>5 ? 5 : x;
}

// unary moves 
const int UNARY_MOVES = 3;

const unsigned long HEAD_LEFT = 1;
const unsigned long HEAD_RIGHT = 2;
inline unsigned long encodeLinkDirection(const unsigned long &head, const unsigned long &mod) {
   assert(mod!=head);
   return mod>head ? HEAD_LEFT : HEAD_RIGHT;
}

inline unsigned long encodeLinkSize(const unsigned long &head, const unsigned long &mod) {
   assert(mod!=head);
   return mod>head ? normalize510(mod-head) : normalize510(head-mod);
}

inline unsigned long encodeLinkDirectionAndSize(const unsigned long &head, const unsigned long &mod) {
   assert(mod!=head);
   assert((normalize510(mod>head?mod-head:head-mod)>>3)==0);
   if (mod>head)
      return (HEAD_LEFT<<3) | normalize510(mod-head);
   else
      return (HEAD_RIGHT<<3) | normalize510(head-mod);
}

#endif

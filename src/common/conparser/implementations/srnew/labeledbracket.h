// Yue Zhang

#ifndef _LABELED_BRACKET_H
#define _LABELED_BRACKET_H

namespace TARGET_LANGUAGE {

class CLabeledBracket {

public:
   unsigned begin;
   unsigned end;
   CConstituent constituent;
};

std::ostream & operator << (std::ostream &os, const CLabeledBracket &lb) {
   os << '<' << lb.begin << ',' << lb.end << ',' << lb.constituent << '>';
}

} // namespace TARGET_LANGUAGE

#endif
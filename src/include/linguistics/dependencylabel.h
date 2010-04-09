/****************************************************************
 *                                                              *
 * dependencylabel.h - the labeled dependency tree              *
 *                                                              *
 * Author: Yue Zhang                                            *
 *                                                              *
 * Computing Laboratory, Oxford. 2008.07                        *
 *                                                              *
 ****************************************************************/

#ifndef _DEPENDENCY_LABEL_H
#define _DEPENDENCY_LABEL_H

#include "dependency.h"

/*==============================================================
 *
 * CLabeledDependencyTreeNode
 *
 *==============================================================*/

class CLabeledDependencyTreeNode : public CDependencyTreeNode {

public:
   string label;

public:
   CLabeledDependencyTreeNode( ) : CDependencyTreeNode(), label("") { }
   CLabeledDependencyTreeNode( const string &w, const string &t, const int &h, const string &l) : CDependencyTreeNode(w, t, h), label(l) { }
   virtual ~CLabeledDependencyTreeNode() {}

public:
   bool operator ==(const CLabeledDependencyTreeNode &item) const { 
      return static_cast<CDependencyTreeNode>(*this) == static_cast<CDependencyTreeNode>(item) && label == item.label; 
   }

};

//==============================================================

inline istream & operator >> (istream &is, CLabeledDependencyTreeNode &node) {
   (is) >> static_cast<CDependencyTreeNode&>(node) >> node.label ;
   if (node.label.empty()) THROW("dependency label is unavailable in the input file");
   return is ;
}

inline ostream & operator << (ostream &os, const CLabeledDependencyTreeNode &node) {
   os << static_cast<const CDependencyTreeNode&>(node) << "\t" << node.label ;
   return os ;
}

/*==============================================================
 *
 * CLabeledDependencyTree
 *
 * Note that we must define the dependency as sentencetemplate
 * in order to use the standard reading and writing functions
 * defined by the sentence template. Inheritance does not work
 * with template because the generic programming idea does not
 * mix well with the object-oriented programming ideologies. 
 *
 *==============================================================*/

typedef CSentenceTemplate<CLabeledDependencyTreeNode> CLabeledDependencyTree ;

#endif

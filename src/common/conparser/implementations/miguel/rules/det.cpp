//"/^(?:NP(?:-TMP|-ADV)?|NAC|NML|NX|X)$/ < (DT=target !< /^(?i:either|neither|both)$/ !$+ DT !$++ CC $++ /^(?:N[MNXP]|CD|JJ|FW|ADJP|QP|RB|PRP(?![$])|PRN)/=det !$++ (/^PRP[$]|POS/ $++ =det !$++ (/''/ $++ =det)))",
bool det1(){

}

//"NP|NP-TMP|NP-ADV < (DT=target < /^(?i:either|neither|both)$/ !$+ DT !$++ CC $++ /^(?:NN|NX|NML)/ !$++ (NP < CC))",
bool det2(){

}

//"NP|NP-TMP|NP-ADV < (DT=target !< /^(?i:either|neither|both)$/ $++ CC $++ /^(?:NN|NX|NML)/)",
bool det3(){

}

//"NP|NP-TMP|NP-ADV < (DT=target $++ (/^JJ/ !$+ /^NN/) !$++CC !$+ DT)",
bool det4(){

}

//"NP|NP-TMP|NP-ADV <<, PRP <- (NP|DT|RB=target <<- all|both|each)", // we all, them all; various structures
bool det5(){

}

//"WHNP < (NP $-- (WHNP=target < WDT))",
 bool det6(){
	 if (node.constituent==PENN_CON_WHNP){
		 CStateNodeList* childsWhnp=node.m_umbinarizedSubNodes;
		 while(childsWhnp!=0){
			 const CStateNode* npChild=childsWhnp->node;
			 if (npChild->constituent==PENN_CON_NP){
				 CStateNodeList* leftSisters=childsWhnp->previous;
				 while(childsWhnp!=0){
					 const CStateNode* whnpTarg=childsWhnp->node;
					 if (whnpTarg->constituent==PENN_CON_WHNP && !(isLinked(&node,whnpTarg))){
						 CStateNodeList* whnpChilds=whnpTarg->m_umbinarizedSubNodes;
						 while(whnpChilds!=0){
							 if ((*words)[whnpChilds->node->lexical_head].tag.code()==PENN_TAG_WDT){
								 CDependencyLabel* label=new CDependencyLabel(STANFORD_DEP_DET);
								 if (buildStanfordLink(label, whnpTarg->lexical_head, node.lexical_head)) {
									 addLinked(&node,whnpTarg);
								     return true;
								 }
							 }
							 whnpChilds=whnpChilds->next;
						 }
					 }
					 childsWhnp=childsWhnp->previous;
				 }
			 }
			 childsWhnp=childsWhnp->next;
		 }
	 }
	 return false;
 }



//"@WHNP|ADVP < (/^(?:NP|NN|CD|RBS)/ $-- DT|WDT|WP=target)"
bool det7(){

}

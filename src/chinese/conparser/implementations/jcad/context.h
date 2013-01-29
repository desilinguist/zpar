// Copyright (C) University of Oxford 2010
/****************************************************************
 *                                                              *
 * context.h - the definition of context                        *
 *                                                              *
 * Author: Yue Zhang                                            *
 *                                                              *
 * Computing Laboratory, Oxford. 2007.12                        *
 *                                                              *
 ****************************************************************/

class CStateNode; 
class CStateItem;

const CConstituent g_noneConstituent(CConstituent::NONE);
const CConstituent g_beginConstituent(CConstituent::SENTENCE_BEGIN);
const CTag g_noneTag(CTag::NONE);
static const CWord g_emptyWord("");
static const CTag g_beginTag(CTag::SENTENCE_BEGIN);

//#define constituent_or_none(x) ((x).is_constituent() ? (x).constituent.code() : CConstituent::NONE)
#define constituent_or_none(x) ((x).constituent.code())
#define _encode_three_constituents(x, y, z) ((x<<CConstituent::SIZE<<CConstituent::SIZE)|(y<<CConstituent::SIZE)|z)
#define _encode_two_constituents(x, y) ((x<<CConstituent::SIZE)|y)
#define _load_char(x) modify ? wrds.replace(x->head_c, x->head_c, &sentence) : wrds.find(x->head_c, x->head_c, &sentence)
#define _load_char_bypos(x) modify ? wrds.replace(x, x, &sentence) : wrds.find(x, x, &sentence)
#define _load_word(x) x->is_partial() ? (modify ? wrds.replace(x->begin_c, x->end_c, &sentence) : wrds.find(x->begin_c, x->end_c, &sentence)) : (modify ? wrds.replace(x->word_head->begin_c, x->word_head->end_c, &sentence) : wrds.find(x->word_head->head_c, x->word_head->end_c, &sentence))
//#define _load_word(x) modify ? wrds.replace(x->word_head->begin_c, x->word_head->end_c, &sentence) : wrds.find(x->word_head->head_c, x->word_head->end_c, &sentence)
#define find_or_replace_word_cache(tmp_start, tmp_end) ( modify ? wrds.replace(tmp_start, tmp_end, &sentence) : wrds.find(tmp_start, tmp_end, &sentence) )
#define refer_or_allocate(x, y, z) {if(!modify) x.refer(&y, &z); else x.allocate(y, z);}


//===============================================================
//
// CContext 
//
//===============================================================

class CContext {
public:
   const CStateNode *s0, *s1, *s2, *s3;
   const CStateNode *s0l, *s0r, *s0u, *s0h;
   const CStateNode *s1l, *s1r, *s1u, *s1h;

   int p1, p2, n0, n1, n2, n3;
   int s0ld, s0rd;
   int s1ld, s1rd;

   CWord s0w, s1w, s2w, s3w;
   CConstituent s0c, s1c, s2c, s3c;
   CTag s0t, s1t, s2t, s3t;
   CWord s0z, s1z, s2z, s3z;

   CWord p1z, p2z, n0z, n1z, n2z, n3z; //addzms


   CWord s0lw, s0rw, s0uw, s1lw, s1rw, s1uw;
   CConstituent s0lc, s0rc, s0uc;
   CTag s0lt, s0rt, s0ut;
   CConstituent s1lc, s1rc, s1uc;
   CTag s1lt, s1rt, s1ut;

   unsigned long s0ldt, s0rdt;
   unsigned long s1ldt, s1rdt;
   CTaggedWord<CTag, TAG_SEPARATOR> s0wt, s1wt;
   CWord s0ldw, s0rdw, s1ldw, s1rdw;

   unsigned long stacksize;

   CCFGSet s0cs1c, s0ts1t, s0cs1cs2c, s0cs0rc, s0ts1ts2t, s0cs0lcs1c, s0cs0ljs1j, s0cs0lc, s0cs1cs1rc, s0js1cs1rj,s1cs1rc;
   CTwoWords s0ws1w;
   CTwoTaggedWords s0wts1wt;

   //tagger
   int start_0;
	int start_1, end_1;
	int start_2, end_2;
	int length_1, length_2;
   CWord word_1, word_2,  first_char_0, first_char_1, last_char_1, last_char_2, two_char, word_1_first_char_0, word_1_last_char_2, three_char;
   CTwoWords word_2_word_1, first_char_1_last_char_1, first_char_0_first_char_1, first_char_0_last_char_1, last_char_1_last_char_2 ;
   CTag tag_1,tag_2;
   CTagSet<CTag, 2> tag_1_tag_2;

   std::vector<CTwoTaggedWords> wt12_collection;
   CTwoTaggedWords wt12_app;


public:

   void load(const CStateItem *item, CWordCache &wrds, const CStringVector &sentence, const bool &modify) {
      stacksize = item->stacksize();
      static unsigned long tmp;
      static int tmp_i;
      static int i, j;
      if(stacksize > 0)
      {
			p1 = item->current_word-1 > 0 ? -1 : item->current_word-1;
			p2 = item->current_word-2 > 0 ? -1 : item->current_word-2;
			n0 = item->current_word >= sentence.size() ? -1 : item->current_word;
			n1 = item->current_word+1 >= sentence.size() ? -1 : item->current_word+1;
			n2 = item->current_word+2 >= sentence.size() ? -1 : item->current_word+2;
			n3 = item->current_word+3 >= sentence.size() ? -1 : item->current_word+3;
			//n1 = -1; n2 = -1; n3 = -1;

			s0 = &(item->node);
			s1 = stacksize<2 ? 0 : &(item->stackPtr->node);
			s2 = stacksize<3 ? 0 : &(item->stackPtr->stackPtr->node);
			s3 = stacksize<4 ? 0 : &(item->stackPtr->stackPtr->stackPtr->node);

			//s0l = s0->is_constituent() ? (s0->single_child()||s0->head_left() ? 0 : s0->left_child) : 0;
			//s0r = s0->is_constituent() ? (s0->single_child()||!s0->head_left() ? 0 : s0->right_child) : 0;
			//s0u = s0->is_constituent() ? (s0->single_child() ? s0->left_child : 0) : 0;

			s0l = s0->single_child()||s0->head_left() ? 0 : s0->left_child;
			s0r = s0->single_child()||!s0->head_left() ? 0 : s0->right_child;
			s0u = s0->single_child() ? s0->left_child : 0;
	//      s0h = s0->is_constituent() ? (s0->single_child()||s0->head_left() ? s0->left_child : s0->right_child) : 0;

			//s1l = s1==0 ? 0 : ( s1->is_constituent() ? (s1->single_child()||s1->head_left() ? 0 : s1->left_child) : 0 );
			//s1r = s1==0 ? 0 : ( s1->is_constituent() ? (s1->single_child()||!s1->head_left() ? 0 : s1->right_child) : 0 );
			//s1u = s1==0 ? 0 : ( s1->is_constituent() ? (s1->single_child() ? s1->left_child : 0) : 0 );
			s1l = s1==0 ? 0 : (s1->single_child()||s1->head_left() ? 0 : s1->left_child);
			s1r = s1==0 ? 0 : (s1->single_child()||!s1->head_left() ? 0 : s1->right_child);
			s1u = s1==0 ? 0 : (s1->single_child() ? s1->left_child : 0);
	//      s1h = s1==0 ? 0 : ( s1->is_constituent() ? (s1->single_child()||s1->head_left() ? s1->left_child : s1->right_child) : 0 );


			s0c.load( constituent_or_none(*s0) );;
			s0w.load(_load_word(s0));
			s0t.load(s0->pos);
			s0wt.load(s0w, s0t);
			s0z.load(_load_char(s0));


			s1c.load( s1==0 ? CConstituent::SENTENCE_BEGIN : constituent_or_none(*s1) );
			s1w.load(s1 == 0 ? g_emptyWord.code() : _load_word(s1));
			s1t.load(s1 == 0 ? g_noneTag.code() : s1->pos);
			s1z.load(s1 == 0 ? g_emptyWord.code() : _load_char(s1));

			s2c.load( s2==0 ? CConstituent::SENTENCE_BEGIN : constituent_or_none(*s2) );
			s2w.load(s2 == 0 ? g_emptyWord.code() : _load_word(s2));
			s2t.load(s2 == 0 ? g_noneTag.code() : s2->pos);
			s2z.load(s2 == 0 ? g_emptyWord.code() : _load_char(s2));

			s3c.load( s3==0 ? CConstituent::SENTENCE_BEGIN : constituent_or_none(*s3) );
			s3w.load(s3 == 0 ? g_emptyWord.code() : _load_word(s3));
			s3t.load(s3 == 0 ? g_noneTag.code() : s3->pos);
			s3z.load(s3 == 0 ? g_emptyWord.code() : _load_char(s3));


			s0lc.load( s0l==0 ? CConstituent::SENTENCE_BEGIN : constituent_or_none(*s0l) );
			s0lw.load(s0l == 0 ? g_emptyWord.code() : _load_word(s0l));
			s0lt = s0l == 0 ? g_noneTag : CTag(s0l->pos);

			s0rc.load(s0r==0 ? CConstituent::SENTENCE_BEGIN : constituent_or_none(*s0r));
			s0rw.load(s0r == 0 ? g_emptyWord.code() : _load_word(s0r));
			s0rt.load(s0r == 0 ? g_noneTag.code() : s0r->pos);

			s0uc.load(s0u==0 ? CConstituent::SENTENCE_BEGIN : constituent_or_none(*s0u));
			s0uw.load(s0u == 0 ? g_emptyWord.code() : _load_word(s0u));
			s0ut = s0u == 0 ? g_noneTag : CTag(s0u->pos);

			s1lc.load( s1l==0 ? CConstituent::SENTENCE_BEGIN : constituent_or_none(*s1l) );
			s1lw.load(s1l == 0 ? g_emptyWord.code() : _load_word(s1l));
			s1lt.load(s1l == 0 ? g_noneTag.code() : s1l->pos);

			s1rc.load(s1r==0 ? CConstituent::SENTENCE_BEGIN : constituent_or_none(*s1r));
			s1rw.load(s1r == 0 ? g_emptyWord.code() : _load_word(s1r));
			s1rt.load(s1r == 0 ? g_noneTag.code() : s1r->pos);

			s1uc.load(s1u==0 ? CConstituent::SENTENCE_BEGIN : constituent_or_none(*s1u));
			s1uw.load(s1u == 0 ? g_emptyWord.code() : _load_word(s1u));
			s1ut.load(s1u == 0 ? g_noneTag.code() : s1u->pos);

			p2z = p2 == -1 ? g_emptyWord : _load_char_bypos(p2);
			p1z = p1 == -1 ? g_emptyWord : _load_char_bypos(p1);
			n0z = n0 == -1 ? g_emptyWord : _load_char_bypos(n0);
			n1z = n1 == -1 ? g_emptyWord : _load_char_bypos(n1);
			n2z = n2 == -1 ? g_emptyWord : _load_char_bypos(n2);
			n3z = n3 == -1 ? g_emptyWord : _load_char_bypos(n3);

			if (modify==false) {
				if (s1!=0) s0ws1w.refer(&s0w, &s1w);
				if (s1!=0) s0wts1wt.refer(&s0wt, &s1wt);
			}
			else {
				if (s1!=0) s0ws1w.allocate(s0w, s1w);
				if (s1!=0) s0wts1wt.allocate(s0wt, s1wt);
			}
			s0cs1c.load(s0c, s1c);
			s0ts1t.load(s0t, s1t);

			s0cs1cs2c.copy(s0cs1c); s0cs1cs2c.add(s2c);

			s0ts1ts2t.copy(s0ts1t); s0ts1ts2t.add(s2t);


			s0cs0rc.load(s0c, s0rc);

			// see comments above
			s0cs0lc.load(s0c, s0lc);
			s0cs0lcs1c.copy(s0cs1c);
			s0cs0lcs1c.add(s0lc);

			s0cs0ljs1j.load(s0c);
			if (s0lc.empty()) s0cs0ljs1j.add(s0lt); else s0cs0ljs1j.add(s0lc);
			if (s1c.empty()) s0cs0ljs1j.add(s1t); else s0cs0ljs1j.add(s1c);

			// s0 slr and s1 -- by presuming that s1 exists!
			s1cs1rc.load(s1c, s1rc);
			s0cs1cs1rc.copy(s0cs1c);
			s0cs1cs1rc.add(s1rc);

			if (s0c.empty()) s0js1cs1rj.load(s0t); else s0js1cs1rj.load(s0c);
			s0js1cs1rj.add(s1c);
			if (s1rc.empty()) s0js1cs1rj.add(s1rt); else s0js1cs1rj.add(s1rc);
      }

      //tagger and segmentation

      //word token feature


		start_0 = item->current_word < sentence.size() ? item->current_word : -1;

		const CStateNode *p0word_state = stacksize == 0 ? 0 : &(item->node);
		const CStateNode *p1word_state = p0word_state == 0 ? 0: p0word_state->word_prev;
		start_1 = p0word_state == 0 ? -1 : p0word_state->begin_c;
		end_1 = p0word_state == 0 ? -1 : p0word_state->end_c;
		length_1 = p0word_state == 0 ? 0 : end_1-start_1+1;

		start_2 = p1word_state == 0 ? -1 : p1word_state->begin_c;
		end_2 = p1word_state == 0 ? -1 : p1word_state->end_c;
		length_2 = p1word_state == 0 ? 0 : end_2-start_2+1;

		word_1 = start_1>=0 ? find_or_replace_word_cache( start_1, end_1 ) : g_emptyWord;
		word_2 = start_2>=1 ? find_or_replace_word_cache( start_2, end_2 ) : g_emptyWord;

		if (length_1>LENGTH_MAX) length_1 = LENGTH_MAX;
		if (length_2>LENGTH_MAX) length_2 = LENGTH_MAX;


	   first_char_0 = start_0>=0 ? find_or_replace_word_cache( start_0, start_0 ) : g_emptyWord ;
	   first_char_1 = start_1>=0 ? find_or_replace_word_cache( start_1, start_1 ) : g_emptyWord;

	   last_char_1 = start_1>=0 ? find_or_replace_word_cache( end_1, end_1 ) : g_emptyWord;
	   last_char_2 = start_2>=0 ? find_or_replace_word_cache( end_2, end_2 ) : g_emptyWord;
	   two_char = start_1>=0&&start_0>=0 ? find_or_replace_word_cache( end_1, start_0 ) : g_emptyWord ;
	   word_1_first_char_0 = start_1>=0&&start_0>=0 ? find_or_replace_word_cache( start_1, start_0 ) : g_emptyWord;
	   word_1_last_char_2 = start_2>=1 ? find_or_replace_word_cache( end_2, end_1 ) : g_emptyWord;
	   three_char = ( length_1==1 && start_2>=1 && start_0>=0 ) ? find_or_replace_word_cache( end_2, start_0 ) : g_emptyWord;


		if (!modify&&start_1>=0) {
			word_2_word_1.refer( &word_1 , &word_2 ) ;
			first_char_1_last_char_1.refer( &first_char_1 , &last_char_1 ) ;
			first_char_0_first_char_1.refer( &first_char_0 , &first_char_1 ) ;
			last_char_1_last_char_2.refer( &last_char_1 , &last_char_2 ) ;
		}
		else {
			word_2_word_1.allocate( word_1, word_2 ) ;
			first_char_1_last_char_1.allocate( first_char_1, last_char_1 ) ;
			first_char_0_first_char_1.allocate( first_char_0, first_char_1 ) ;
			last_char_1_last_char_2.allocate( last_char_1, last_char_2 ) ;
		}

	      // about::cout the tags
		tag_1.load(p0word_state == 0 ? g_beginTag.code() : p0word_state->pos);
		tag_2.load(p1word_state == 0 ? g_beginTag.code() : p1word_state->pos);



		tag_1_tag_2.load( encodeTags(tag_1, tag_2) );

		static CTwoTaggedWords wt12;
		CTaggedWord<CTag, TAG_SEPARATOR> wt1, wt2;
      for (int j=0; j<length_1-1; ++j) {
         wt1.load(find_or_replace_word_cache(start_1+j, start_1+j), tag_1);
         wt2.load(last_char_1);//
         if (!modify) { wt12.refer(&wt1, &wt2); } else { wt12.allocate(wt1, wt2); }
         wt12_collection.push_back(wt12);
      }



      wt1.load(first_char_0, tag_1);
		wt2.load(first_char_1);
		refer_or_allocate(wt12_app, wt1, wt2);








      return;
   }
};
                           

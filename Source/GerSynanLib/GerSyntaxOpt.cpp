#include  "GerSynan.h"
#include  "GerSentence.h"
#include  "GerSyntaxOpt.h"
#include  "GerThesaurus.h"
#include  "../StructDictLib/TempArticle.h"
#include "../SimpleGrammarLib/SimpleGrammar.h"


const int gSyntaxGroupTypesCount = 32;
const char gSyntaxGroupTypes [gSyntaxGroupTypesCount][30] = 
{
	"DET_ADJ_SUB", "WWW", "KEYB", "PREP_NOUN", "MODIF_ADJ", 
	"SIMILAR_ADJ", "SIMILAR_NOMEN",	"SIMILAR_ADV", "GEN_NOUN",
	"DIR_OBJ", "NP", "NOUN_ADJ", "VERB_MODIF", "ADJ_SUB",
	"FAMIL", "BERUF_NP", "GENIT_NP", "SIMILAR_NP",
	"APPOSITION", "DISKONT_KONJ", "DISKONT_KONJ_REL", "EXPR",
	"SUBJ", "SPNAME", "TITLE", "PNAME", 
	"PERSON", "APPOS", "ADDITION", "MODIF_NUMER", "GEOGR",
	"GENIT_PRE"
};

CSentence* CGerSyntaxOpt::NewSentence() const {
	return new CGerSentence(this);
};


CGerSyntaxOpt :: CGerSyntaxOpt (MorphLanguageEnum langua) : CSyntaxOpt(langua)
{
	m_piGramTab = new CGerGramTab();

	m_SimilarNPGroupType = GetSyntaxGroupOrRegister("NP_SIMIL");
	m_PluralMask = _QM(gPlural);
	m_SingularMask = _QM(gSingular);
	m_IndeclinableMask = 0;
	m_GenNounGroupType =  gGEN_NOUN;
	m_PrepNounGroupType=    gPREP_NOUN;
	m_DirObjGroupType=  gDIR_OBJ;
	m_NPGroupType=  gNOUN_GR;
	m_NounAdjGroupType=  gNOUN_ADJ;
	m_NameGroupType = gFAMILIE_GROUP;
	m_OborotGroupType = gOBOROTS;
	m_WWWGroupType = gWEB_ADDR;
	m_KEYBGroupType = gKEYB;
	m_SubjRelation = gSUBJ;
	m_RusParenthesis = UnknownPartOfSpeech;
	m_Preposition = gPRP;
	m_Conjunction = gKON;


	m_DisruptConjRelation = gDISRUPT_CONJ_RELATION;
	m_DisruptConjGroupType = gDISKONT_KONJ;
	m_InfintiveClauseType = INFINITIVSATZ_T;


	for (size_t i=0; i < gSyntaxGroupTypesCount; i++)
		m_SyntaxGroupTypes.push_back(gSyntaxGroupTypes[i]);

}

CLemmatizer *CGerSyntaxOpt::NewLemmatizer() const {
	return new CLemmatizerGerman();
};

COborDic * CGerSyntaxOpt::NewOborDic(const CSyntaxOpt* opt)  {
	return new CGerOborDic(opt);
};

CThesaurusForSyntax* CGerSyntaxOpt::NewThesaurus(const CSyntaxOpt* opt) {
	return new CGerThesaurusForSyntax(opt);
};


void CGerSyntaxOpt::DestroyOptions ()
{
	CSyntaxOpt::DestroyOptions();
};

static std::string GetSyntaxFilePath()
{
	return GetRmlVariable()+"/Dicts/GerSynan/";
};



bool CGerSyntaxOpt :: InitOptionsLanguageSpecific()
{
	if (!m_piGramTab->LoadFromRegistry()) {
		return false;
	}

	//  reading adjektives
	std::string strFileName = GetSyntaxFilePath()+"adj_prp.txt";
	if (!ReadListFile (strFileName, m_AdjPrp))
		return false;
	
	// reading formats
	strFileName = GetSyntaxFilePath()+"gformats.grm";
	m_FormatsGrammar.m_Language = morphGerman;
	m_FormatsGrammar.m_pGramTab = GetGramTab();
	m_FormatsGrammar.m_SourceGrammarFile = strFileName;
	if (!LoadGrammarForGLR(m_FormatsGrammar, true, false))
	{
		ErrorMessage(  Format("Cannot load %s\n", strFileName.c_str()));
		return false;
	};
	
	
	return true;
}




bool CGerSyntaxOpt::is_firm_group(int GroupType) const
{
	return (		(GroupType == gWEB_ADDR ) 
				||	(GroupType == gKEYB ) 
				||	(GroupType == gOBOROTS )
			);
}

bool CGerSyntaxOpt::IsGroupWithoutWeight(int GroupType, const char* cause) const
{
	return false;
};

bool CGerSyntaxOpt::IsSimilarGroup (int type) const
{
	return false;
};

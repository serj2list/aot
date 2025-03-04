#pragma once 

#include "Homonym.h"
#include "PlmLine.h"
#include "gra_descr.h"

class CLemWord  
{
	// graphematical descriptors in one std::string (without some binary flags that could be restored by CLemWord::BuildGraphemDescr() )
	

    uint64_t   m_GraDescrs;
    int		ProcessGraphematicalDescriptors(const char* LineStr);
public:

	// ======= Graphematics ======================	
    std::string  m_UnparsedGraphemDescriptorsStr;

	// input word form
	std::string m_strWord;

	// input word uppercase form 
	std::string m_strUpperWord;

	// is graphematical space 
	bool m_bSpace;


	// graphematical descriptor RLE or LLE
	bool m_bWord; 

	// a single comma
	bool m_bComma;

	// a single hyphen
	bool m_bDash;

	// graphematical register 
	RegisterEnum m_Register;

	//  offset in the graphematcil buffer (= in the input file for text files)
	int	 m_GraphematicalUnitOffset;

    int m_TokenLengthInFile;

	// true, if this word has a space before ot it is at the beginning of the sentence.
	bool	m_bHasSpaceBefore;

	//  true if the word was deleted and should be ignored 
	bool	m_bDeleted;

	// is morphologically predicted
	bool m_bPredicted;


	CLemWord(); 
	
	void DeleteOborotMarks();
	bool AddNextHomonym(const char* strPlmLine);
	bool ProcessPlmLineForTheFirstHomonym(const char* strPlmLine, MorphLanguageEnum langua, int& OborotNo);
	bool HasDes(Descriptors g) const;
    void DelDes(Descriptors g);
    void AddDes(Descriptors g);
	
	void	Reset();
	
	
	
	void	SetWordStr (std::string NewValue, MorphLanguageEnum langua);

	bool	FindLemma(std::string strLemma) const;	
	int		GetHomonymByPOS(BYTE POS) const;
    bool    HasPos(BYTE POS) const;
    bool    HasGrammem(BYTE Grammem) const;
	int		GetHomonymByGrammem(BYTE grammem) const;
	int		GetHomonymByPOSandGrammem(BYTE POS, BYTE grammem) const;
    int     GetHomonymByPosesandGrammem(part_of_speech_mask_t Poses, BYTE grammem) const;
	bool	IsWordUpper(const char* s)	const  {return m_strUpperWord == s; };

	void	SetAllOtherHomsDel(int iHom);
	
	


    virtual size_t	GetHomonymsCount() const	= 0; 
	virtual const CHomonym* GetHomonym(int i) const	 = 0;
	virtual CHomonym* GetHomonym(int i) = 0;
	virtual void EraseHomonym(int iHom) = 0;;
    virtual CHomonym* AddNewHomonym() = 0;;

	void DeleteMarkedHomonymsBeforeClauses();
	void SetHomonymsDel(bool Value);

	bool IsFirstOfGraPair(EGraPairType type) const;
	bool IsFirstOfGraPair() const;
	bool IsSecondOfGraPair(EGraPairType type) const;
	bool IsSecondOfGraPair() const;


	int		GetOborotNo() const;
	bool	HasOborot1() const;
	bool	HasOborot2() const;
	bool	IsInOborot() const;
	bool	CanBeSynNoun() const;
	void	KillHomonymOfPartOfSpeech(int iPartOfSpeech);
    virtual void InitLevelSpecific(CHomonym* ) {};
    std::string  GetPlmStr (const CHomonym* pHomonym, bool bFirstHomonym)  const;
    std::string  GetDebugString(const CHomonym* pHomonym, bool bFirstHomonym)  const;
    std::string BuildGraphemDescr ()  const;
    part_of_speech_mask_t GetPoses() const;
    uint64_t   GetGrammems() const;
    bool    HasAnalyticalBe() const;
};

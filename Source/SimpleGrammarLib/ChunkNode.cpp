// QNode.cpp: implementation of the CQNode class.
//
//////////////////////////////////////////////////////////////////////
#include  "SimpleGrammarLib.h"
#include "ChunkNode.h"



extern std::string ConvertToPlural(const CAgramtab*,  const std::string&);
extern std::string CommonCaseNumberGender(const CAgramtab*,  const std::string&, const std::string&);
extern std::string WeakDeclAssignFunction2(const CAgramtab*,  const std::string&, const std::string&);
extern std::string WeakDeclAssignFunction3(const CAgramtab*,  const std::string&, const std::string&, const std::string&);
extern std::string MixedDeclAssignFunction2(const CAgramtab*,  const std::string&, const std::string&);
extern std::string MixedDeclAssignFunction3(const CAgramtab*,  const std::string&, const std::string&, const std::string&);
extern std::string StrongDeclAssignFunction(const CAgramtab*,  const std::string&, const std::string&);
extern std::string RussianCaseNumberGender(const CAgramtab* pGramTab,  const std::string& adj, const std::string& common_noun_grm, const std::string& noun);
extern bool	 HasOnlyOneCase(const CAgramtab*,  const std::string&, const part_of_speech_mask_t&, const uint64_t&);
extern bool	 HasGrammem(const CAgramtab*,  const std::string&, const part_of_speech_mask_t&, const uint64_t& );
extern bool	 HasOneGrammem(const CAgramtab*,  const std::string&, const part_of_speech_mask_t&, const uint64_t& );
extern std::string CommonCase(const CAgramtab*,  const std::string&, const std::string&);


bool set_true() {	return true; };
bool set_false() {	return false; };
bool is_true(const bool b) {	return b; };
bool is_false(const bool b) {	return !b; };






const size_t PossibleAssignFunc0Count = 2;
std::pair<std::string, AssignType0*> PossibleAssignFunc0[PossibleAssignFunc0Count] = {
	std::make_pair(std::string("true"), (AssignType0*)set_true),
	std::make_pair(std::string("false"), (AssignType0*)set_false)
};


const size_t PossibleCheckFunc1Count = 3;
std::pair<std::string, CheckType1*> PossibleCheckFunc1[PossibleCheckFunc1Count] = {
	std::make_pair(std::string("true"), (CheckType1*)is_true),
	std::make_pair(std::string("false"), (CheckType1*)is_false),
	std::make_pair(std::string("is_atomic"), (CheckType1*)is_true)
};

const size_t PossibleCheckFunc3Count = 3;
std::pair<std::string, CheckType3*> PossibleCheckFunc3[PossibleCheckFunc3Count] = {
	std::make_pair(std::string("has_only_case"), (CheckType3*)HasOnlyOneCase),
	std::make_pair(std::string("has_grammem"), (CheckType3*)HasGrammem),
	std::make_pair(std::string("has_one_grm"), (CheckType3*)HasOneGrammem)
};

const size_t PossibleFunc1Count = 1;
std::pair<std::string, AssignType1*> PossibleFunc1[PossibleFunc1Count] = {
	std::make_pair(std::string("convert_to_plural"), (AssignType1*)ConvertToPlural)
};

const size_t PossibleFunc2Count = 5;
std::pair<std::string, AssignType2*> PossibleFunc2[PossibleFunc2Count] = {
	std::make_pair(std::string("common_case_number_gender"), (AssignType2*)CommonCaseNumberGender),
	std::make_pair(std::string("weak_decl"), (AssignType2*)WeakDeclAssignFunction2),
	std::make_pair(std::string("mixed_decl"), (AssignType2*)MixedDeclAssignFunction2),
	std::make_pair(std::string("strong_decl"), (AssignType2*)StrongDeclAssignFunction),
	std::make_pair(std::string("common_case"), (AssignType2*)CommonCase)
};

const size_t PossibleFunc3Count = 3;
std::pair<std::string, AssignType3*> PossibleFunc3[PossibleFunc3Count] = 
{
		std::make_pair(std::string("weak_decl"), (AssignType3*)WeakDeclAssignFunction3 ), 
		std::make_pair(std::string("mixed_decl"), (AssignType3*)MixedDeclAssignFunction3),
		std::make_pair(std::string("case_number_gender"), (AssignType3*)RussianCaseNumberGender)
};






bool CRuleFeature::InitFuncName(std::string s)
{
	m_FuncNameStr = s;
	if (m_Type == foeAssignement)
		switch (m_RightItems.size()) {
			case 0:	
				{
					for (size_t i=0; i< PossibleAssignFunc0Count; i++)
						if (s == PossibleAssignFunc0[i].first)
						{
							m_pAssign0 = PossibleAssignFunc0[i].second;
							m_FuncType = ffeAssign0;
							return true;
						};
				}
			case 1:	
				{
					for (size_t i=0; i< PossibleFunc1Count; i++)
						if (s == PossibleFunc1[i].first)
						{
							m_pAssign1 = PossibleFunc1[i].second;
							m_FuncType = ffeAssign1;
							return true;
						};
				}
			case 2:	
				{
					for (size_t i=0; i< PossibleFunc2Count; i++)
						if (s == PossibleFunc2[i].first)
						{
							m_pAssign2 = PossibleFunc2[i].second;
							m_FuncType = ffeAssign2;
							return true;
						};
				}
			case 3:	
				{
					for (size_t i=0; i< PossibleFunc3Count; i++)
						if (s == PossibleFunc3[i].first)
						{
							m_pAssign3 = PossibleFunc3[i].second;
							m_FuncType = ffeAssign3;
							return true;
						};
				}
		}
	else
	{
		assert  (m_Type == foeCheck);
		switch (m_RightItems.size()) {
			case 1:	
				{
					for (size_t i=0; i< PossibleCheckFunc1Count; i++)
						if (s == PossibleCheckFunc1[i].first)
						{
							m_pCheck1 = PossibleCheckFunc1[i].second;
							m_FuncType = ffeCheck1;
							return true;
						};
				}
			case 2:	
				{
					for (size_t i=0; i< PossibleCheckFunc3Count; i++)
						if (s == PossibleCheckFunc3[i].first)
						{
							m_pCheck3 = PossibleCheckFunc3[i].second;
							m_FuncType = ffeCheck3;
							return true;
						};
				}
		}
		
	};


	return false;
		
		
};

//===========   CRuleFeature =========================
static bool FeatureToId(std::string s, CAttribAndId& Result)
{
	if (s.length() < 3) return false;
	if (s[0] != '$') return false;
	if (!isdigit((BYTE) s[1]) ) return false;
	if (s[2] != '.') return false;
	Result.m_Id = s[1]-'0';
	Result.m_AttribName = s.substr(3);
	return true;
	
};

static bool InitAttribValue(const CAgramtab* pGramTab, std::string s, CAttribAndId& Result)
{
	Result.m_MorphPattern.m_GrmAttribute = s;
	std::string ErrorStr;
	if (!Result.m_MorphPattern.Init(pGramTab, ErrorStr))
		return false;
	return true;
	
};

bool CRuleFeature::AddFeatureValue(const CAgramtab* pGramTab, std::string s)
{
	CAttribAndId A;
	if (! InitAttribValue(pGramTab, s, A) ) return false;
	m_RightItems.push_back(A);
	return true;
};

bool CRuleFeature::AddFeatureArgument(std::string s)
{
	CAttribAndId A;
	if (! FeatureToId( s, A) ) return false;
	m_RightItems.push_back(A);
	
	return true;
};

std::string CRuleFeature::InitAssignement(std::string s, std::string func_name)
{
	CAttribAndId A;
	if (! FeatureToId(s, A) ) "Bad item id:"+s;
	m_LeftItem = A;
	m_Type = foeAssignement;
	if (!InitFuncName(func_name))
	{
		return func_name + " is a wrong assignement function or wrong number of arguments";
	};
	
	return "";
};

std::string CRuleFeature::InitCheck(std::string func_name)
{
	m_Type = foeCheck;
	if (!InitFuncName(func_name))
	{
		return func_name + " is a wrong check function or wrong number of arguments";
	};
	return "";
};

//===========   CNodeAttribute =========================
CNodeAttribute::CNodeAttribute(std::string Name,	std::string Value)
{
	m_Name = Name;
	m_Value = Value;
};

std::string CNodeAttribute::GetStr() const
{
	return Format("%s=\"%s\"", m_Name.c_str(), m_Value.c_str());
};

CNodeAttributes::~CNodeAttributes()
{
	for (auto& a : m_Items)
	{
		delete a;
	}
};

//===========   CChunkNode =========================

CChunkNode::CChunkNode() : m_AtomicName("")
{
	m_pAttributes = 0;
	m_RefCount = 1;
};


CChunkNode::~CChunkNode()
{
	if (m_pAttributes)
		delete m_pAttributes;
};

void CChunkNode::CreateAtomic(std::string AtomicName, CNodeAttributes* pAttributes)
{
	m_AtomicName = AtomicName;
	m_pAttributes = pAttributes;
};

std::string CChunkNode::GetStr() const
{
	std::string Result = m_AtomicName;
	Result += " ";
	if (m_pAttributes)
		for (const auto& a : m_pAttributes->m_Items)
		{
			Result += a->GetStr();
			Result += " ";
		};
	Trim(Result);
	return std::string("[")+Result+std::string("]");
};

//===========   CChunkSequenceNode =========================

void CChunkSequenceNode::Create(CChunkNode* child1)
{
	assert (m_Children.empty());
	m_Children.push_back(child1);
	child1->m_RefCount++;
}

const std::vector<CChunkNode*>& CChunkSequenceNode::GetChildren() const
{
	return m_Children;
}

void CChunkSequenceNode::AddChild(CChunkNode* child1)
{
	m_Children.push_back(child1);
	child1->m_RefCount++;
}

void CChunkSequenceNode::DeleteLastChild()
{
	assert (!m_Children.empty());
	assert(m_Children.back()->m_RefCount > 1);
	m_Children.back()->m_RefCount--;
	m_Children.erase(m_Children.end() - 1);
}

CChunkSequenceNode* CChunkSequenceNode::Clone() const
{
	CChunkSequenceNode* pNew = new CChunkSequenceNode;
	if (!pNew) return pNew;

	pNew->m_Children = m_Children;

	for (int i = 0; i < m_Children.size(); i++)
		m_Children[i]->m_RefCount++;

	return pNew;
}

void CChunkSequenceNode::Destroy()
{
	for (int i = 0; i < m_Children.size(); i++)
	{
		CChunkNode* pNode= m_Children[i];
		if (pNode->m_RefCount == 1)
			delete pNode;
		else
			pNode->m_RefCount--;
	};
	m_RefCount--;
};

CChunkSequenceNode::~CChunkSequenceNode()
{
	assert (m_RefCount == 1);
	Destroy();
	
};

std::string CChunkSequenceNode::GetStr() const
{
	std::string Result;
	for (int i = 0; i < m_Children.size(); i++)
	{
		Result += m_Children[i]->GetStr();
		Result += " ";
	};
	Trim(Result);
	if (m_Children.size() > 1)
		Result = std::string("(")+Result+std::string(")");
	return Result;
};

//===========   CChunkRule =========================

CChunkRule::CChunkRule() : m_SourceFileName("")
{
	m_pLeftHand = 0;
 
};

CChunkRule::~CChunkRule()
{
	if (m_pLeftHand)
	{
		assert (m_pLeftHand->m_RefCount == 1);
		delete m_pLeftHand;
	};

	for (std::list<CChunkSequenceNode*>::iterator it = m_pRightHand.begin(); it != m_pRightHand.end(); it++)
	{
		CChunkSequenceNode* pNode = *it;

		if (pNode->m_RefCount == 1)
			delete pNode;
		else
			pNode->m_RefCount--;
	};
	
};

CChunkRule::CChunkRule(const CChunkRule& X)
{
	*this = X;
};

CChunkRule& CChunkRule::operator= (const CChunkRule& X)
{
	m_pLeftHand = X.m_pLeftHand;
	m_pRightHand = X.m_pRightHand;
	m_FeatureExprs = X.m_FeatureExprs;
	m_SourceLineNo = X.m_SourceLineNo;
	m_SourceFileName = X.m_SourceFileName;
	return *this;
};


void CChunkRule::SetLeftHandNode(CChunkNode* child1)
{
	m_pLeftHand = child1;
	m_pLeftHand->m_RefCount++;
}

bool CChunkRule::CreateRule(CChunkNode* child1)
{
	assert (m_pRightHand.empty());
	CChunkSequenceNode* pNew = new CChunkSequenceNode;
	if (!pNew) return false;
	pNew->Create(child1);
	m_pRightHand.push_back(pNew);
	
	return true;

}



bool CChunkRule::AddRightHandMultipleNode(CChunkRule* pRule, ChunkOperationEnum Type)
{
	assert (!pRule->m_pRightHand.empty());
	assert (!m_pRightHand.empty());

	if	(		(Type == stSequence) 
		)
	{
		std::list<CChunkSequenceNode*> NewList;
		for (std::list<CChunkSequenceNode*>::iterator it1 = m_pRightHand.begin(); it1 != m_pRightHand.end(); it1++)
		{
			CChunkSequenceNode* pNode  = *it1;
			for (std::list<CChunkSequenceNode*>::iterator it2 = pRule->m_pRightHand.begin(); it2 != pRule->m_pRightHand.end(); it2++)
			{
				CChunkSequenceNode* NewNode = pNode->Clone();
				CChunkSequenceNode* NodeToAdd = *it2;
				NewNode->AddChild(NodeToAdd);
				NewList.push_back(NewNode);
			}
			//assert (pNode->m_RefCount > 1);
			delete pNode;
		};
		m_pRightHand = NewList;
	}
	else
	{
		assert (Type == stOr);
		CChunkSequenceNode* first = *m_pRightHand.begin();
		
		for (std::list<CChunkSequenceNode*>::iterator it = m_pRightHand.begin(); it != m_pRightHand.end(); it++)		
			if ((*it)->GetChildren().back() !=  first->GetChildren().back())
				break;
			else
			{
				for (std::list<CChunkSequenceNode*>::iterator it2 = pRule->m_pRightHand.begin(); it2 != pRule->m_pRightHand.end(); it2++)
				{
					// clone node
					CChunkSequenceNode* CloneNode = first->Clone();
					CloneNode->DeleteLastChild();
					CloneNode->AddChild(*it2);
					m_pRightHand.push_back(CloneNode);
				}
			};


	};

	delete pRule;
	return true;
}

bool CChunkRule::MakeLastNodeFacultative()
{
	if (m_pRightHand.empty()) return false;
	
	size_t Count = m_pRightHand.size();
	std::list<CChunkSequenceNode*>::iterator it = m_pRightHand.begin();
	for (size_t i=0; i < Count; it++,i++)		
	{
		CChunkSequenceNode* CloneNode = (*it)->Clone();
		CloneNode->DeleteLastChild();
		m_pRightHand.push_back(CloneNode);
	};

	return true;
};

bool CChunkRule::EncloseNodes()
{
	for (std::list<CChunkSequenceNode*>::iterator it = m_pRightHand.begin(); it != m_pRightHand.end(); it++)
	{
		CChunkSequenceNode* pNode  = *it;
		if (pNode->GetChildren().size() > 1)
		{
			CChunkSequenceNode* pNew = new CChunkSequenceNode;
			if (!pNew) return false;
			pNew->Create(pNode);
			*it = pNew;
		};
	};
	return true;
};

std::string CChunkRule::GetStr() const
{
	std::string Result = m_pLeftHand->GetStr();
	Result += " -> ";
	
	for (std::list<CChunkSequenceNode*>::const_iterator it = m_pRightHand.begin(); it != m_pRightHand.end(); it++)
	{
		Result += (*it)->GetStr();
		Result += " | ";
	};
	Trim(Result);
	Result.erase(Result.length()-1); 
	Trim(Result);

	return Result;
};

void CChunkRule::AddRuleFeatute(CRuleFeature* pFeature)
{
	m_FeatureExprs.push_back(*pFeature);
	delete pFeature;
};

//===========   CChunkGrammar =========================

CChunkGrammar::~CChunkGrammar()
{
	for (std::list<CChunkRule*>::iterator it = m_Rules.begin(); it != m_Rules.end(); it++)
	{
		CChunkRule* pNode = *it;
		delete pNode;
	};
};

std::string CChunkGrammar::GetStr() const
{
	std::string Result = "";
	for (std::list<CChunkRule*>::const_iterator it = m_Rules.begin(); it != m_Rules.end(); it++)
	{
		Result += (*it)->GetStr();
		Result += ";\n";
	};
	return Result;
};

void CChunkGrammar::AddRules(const CChunkGrammar& R)
{
	m_Rules.insert(m_Rules.end(), R.m_Rules.begin(), R.m_Rules.end());
};

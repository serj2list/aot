#include "stdafx.h"
#include "HierarchyHolder.h"
#include "../StructDictLib/TempArticle.h"


//===========================================
//=========  CStringRelation ================
//===========================================

CStringRelation::CStringRelation(const std::string& Source, const std::string& Target)
{
	m_Source = Source;
	m_Target = Target;
};
bool CStringRelation::operator == (const CStringRelation& X) const
{
	return		(X.m_Source == m_Source)
		&& (X.m_Target == m_Target);
};

bool CStringRelation::operator < (const CStringRelation& X) const
{
	return	((X.m_Source == m_Source)
		&& (m_Target < X.m_Target)
		)
		|| (m_Source < X.m_Source);
};



//===========================================
//=========  CHierarchyNode ================
//===========================================

CHierarchyNode::CHierarchyNode(const std::string& Name, const CNodePlace& p)
{
	m_Name = Name;
	m_Point = p;
};
CHierarchyNode::CHierarchyNode(const std::string& Name)
{
	m_Name = Name;
};
const std::string& CHierarchyNode::GetName() const
{
	return m_Name;
};

//===========================================
//=========  CHierarchyRelation ================
//===========================================

CHierarchyRelation::CHierarchyRelation()
{

};

CHierarchyRelation::CHierarchyRelation(size_t _node1, size_t _node2, std::string _name, bool _bWork)
{
	node1 = _node1;
	node2 = _node2;
	name = _name;
	bWork = _bWork;
};
bool CHierarchyRelation::operator ==  (const CHierarchyRelation& X) const
{
	return (X.node1 == node1) && (X.node2 == node2);
};



//===========================================
//=========  CHierarchyHolder ================
//===========================================

CHierarchyHolder::CHierarchyHolder(CRossHolder* pRossDoc) :m_pRossDoc(pRossDoc)
{

};


CHierarchyHolder::~CHierarchyHolder()
{
	//ml_TRACE("destructor HierarchyHolder");
};

void CHierarchyHolder::WriteToRoss(std::string Entry)
{

	std::string EntryName = (m_Type == SemFet) ? _R("_иерархСХ") : _R("_иерархСО");
	uint16_t UnitNo = GetRoss()->LocateUnit(EntryName.c_str(), 1);
	if (UnitNo != ErrUnitNo)
		GetRoss()->DelUnit(GetRoss()->GetUnits().begin() + UnitNo);
	UnitNo = GetRoss()->InsertUnit(EntryName.c_str(), 1);
	CTempArticle A;
	A.m_pRoss = const_cast<CDictionary*>(GetRoss());
	A.ReadFromDictionary(UnitNo, true, false);
	A.ReadFromUtf8String(Entry.c_str());
	A.WriteToDictionary();
};

void CHierarchyHolder::MySerialize(bool IsStoring, bool WithoutView)
{


	if (IsStoring)
	{
		assert(!WithoutView);
		std::string NodesStr;

		for (int i = 0; i < Nodes.size(); i++)
		{
			std::string S = Format("%s %i %i\r\n", Nodes[i].GetName().c_str(), Nodes[i].m_Point.y, Nodes[i].m_Point.x);
			NodesStr += "NODE = " + S;
		};

		std::string RelationsStr;
		for (int i = 0; i < Relations.size(); i++)
		{
			std::string S = Format("%s %s\r\n", Nodes[Relations[i].node1].GetName().c_str(), Nodes[Relations[i].node2].GetName().c_str());
			RelationsStr += "ISA = " + S;
		};

		// запись в РОСС
		WriteToRoss(NodesStr + RelationsStr);
	}
	else
	{

		ReadFromRoss(WithoutView);

	}

};



void CHierarchyHolder::ReadFromRoss(bool WithoutView)
{

	// TODO: add loading code here
	Nodes.clear();
	Relations.clear();
	std::string EntryName = (m_Type == SemFet) ? _R("_иерархСХ") : _R("_иерархСО");
	uint16_t UnitNo = GetRoss()->LocateUnit(EntryName.c_str(), 1);
	if (UnitNo == ErrUnitNo) {
		ErrorMessage(Format("cannot read %s", convert_to_utf8(EntryName, morphRussian).c_str()));
		return;
	}

	CTempArticle A;

	A.m_pRoss = const_cast<CDictionary*>(GetRoss());
	A.ReadFromDictionary(UnitNo, true, false);
	BYTE NodeFieldNo = GetRoss()->GetFieldNoByFieldStr("NODE");
	BYTE RelFieldNo = GetRoss()->GetFieldNoByFieldStr("ISA");

	if ((NodeFieldNo == ErrUChar) || (RelFieldNo == ErrUChar)) return;
	for (long i = 0; i < A.GetCortegesSize(); i++)
	{
		TCortege C = A.GetCortege(i);
		if ((C.m_FieldNo == NodeFieldNo)
			&& (C.m_DomItemNos[0] != -1)
			&& (C.m_DomItemNos[1] != -1)
			&& (C.m_DomItemNos[2] != -1)
			)
		{
			CNodePlace p;
			p.y = atoi(m_pRossDoc->GetDomItemStrInner(C.m_DomItemNos[1]));
			p.x = atoi(m_pRossDoc->GetDomItemStrInner(C.m_DomItemNos[2]));
			CreateNode(m_pRossDoc->GetDomItemStrInner(C.m_DomItemNos[0]), p, WithoutView);
		};
		if ((C.m_FieldNo == RelFieldNo)
			&& (C.m_DomItemNos[0] != -1)
			&& (C.m_DomItemNos[1] != -1)
			)
			CreateRelation(FindNodeByName(m_pRossDoc->GetDomItemStrInner(C.m_DomItemNos[0])),
				FindNodeByName(m_pRossDoc->GetDomItemStrInner(C.m_DomItemNos[1])),
				true, WithoutView);
	};

}


BYTE CHierarchyHolder::GetDomNo() const
{
	return GetRoss()->GetDomenNoByDomStr((m_Type == SemRel) ? "D_SEM_REL" : "D_SF");
};

int CHierarchyHolder::IncomingEdgesCount(int NodeNo) const
{
	int Result = 0;

	for (size_t i = 0; i < Relations.size(); i++)
		if (Relations[i].node2 == NodeNo)
			Result++;

	return Result;
};

size_t CHierarchyHolder::FindNodeByName(const char* name) const
{
	size_t i = 0;
	for (; i < Nodes.size(); i++)
		if (Nodes[i].GetName() == name)
			break;
	return i;
};


void CHierarchyHolder::SetHierarchyTransitiveClosure()
{
	// TODO: Add your implementation code here
	m_TransitiveRels.clear();
	for (long i = 0; i < Relations.size(); i++)
		m_TransitiveRels.push_back(CStringRelation(Nodes[Relations[i].node1].GetName(), Nodes[Relations[i].node2].GetName()));


	// установка транзитивного замыкания
	size_t PrevCount = 0;
	while (PrevCount < m_TransitiveRels.size())
	{

		PrevCount = m_TransitiveRels.size();

		for (size_t i = 0; i < m_TransitiveRels.size(); i++)
			for (size_t k = 0; k < m_TransitiveRels.size(); k++)
				if (m_TransitiveRels[i].m_Target == m_TransitiveRels[k].m_Source)
				{
					CStringRelation R(m_TransitiveRels[i].m_Source, m_TransitiveRels[k].m_Target);
					if (find(m_TransitiveRels.begin(), m_TransitiveRels.end(), R) == m_TransitiveRels.end())
						m_TransitiveRels.push_back(R);
				};

	};

	sort(m_TransitiveRels.begin(), m_TransitiveRels.end());

}

void CHierarchyHolder::CreateNode(const char* Name)
{
	CHierarchyNode N(Name);
	N.ItemNo = GetRoss()->GetItemNoByItemStr(Name, GetDomNo());;
	Nodes.push_back(N);
};

void CHierarchyHolder::CreateRelation(int node1, int node2, bool bWork)
{
	CHierarchyRelation R;
	R.node1 = node1;
	R.node2 = node2;
	R.bWork = bWork;
	Relations.push_back(R);

};

void CHierarchyHolder::CreateNode(const char* name, CNodePlace p, bool WithoutView)
{
	CreateNode(name);
};

void CHierarchyHolder::CreateRelation(int node1, int node2, bool bWork, bool WithoutView)
{
	CreateRelation(node1, node2, bWork);
};


//==========================================
//===========                ===============
//==========================================
struct LessStringRelation {

	bool  operator () (const CStringRelation& X, const std::string& Source) const
	{
		return X.m_Source < Source;
	};
	bool  operator () (const std::string& Source, const CStringRelation& X) const
	{
		return Source < X.m_Source;
	};
	bool  operator () (const CStringRelation& X1, const CStringRelation& X2) const
	{
		return X1.m_Source < X2.m_Source;
	};
};

bool IsEqualOrHigherInHierarchy(CRossHolder* HostRoss, long HostItemNo, CRossHolder* SlaveRoss, long SlaveItemNo, CHierarchyHolder* pHierarchyDoc)
{
	std::string Host = HostRoss->GetDomItemStrInner(HostItemNo);
	std::string Slave = SlaveRoss->GetDomItemStrInner(SlaveItemNo);

	if (Host == Slave) return true;

	std::vector<CStringRelation>::iterator It = lower_bound(pHierarchyDoc->m_TransitiveRels.begin(), pHierarchyDoc->m_TransitiveRels.end(), Host, LessStringRelation());

	for (; (It < pHierarchyDoc->m_TransitiveRels.end()) && (It->m_Source == Host); It++)
		if (It->m_Target == Slave)
			return true;

	return false;
};

void  IncludeLowerInHierarchy(CHierarchyHolder* pHierarchyDoc, std::vector<std::string>& SemFets)
{
	for (int i = 0; i < SemFets.size(); i++)
		for (int k = 0; k < pHierarchyDoc->m_TransitiveRels.size(); k++)
			if ((SemFets[i] == pHierarchyDoc->m_TransitiveRels[k].m_Source)
				&& (find(SemFets.begin(), SemFets.end(), pHierarchyDoc->m_TransitiveRels[k].m_Target) == SemFets.end())
				)
				SemFets.push_back(pHierarchyDoc->m_TransitiveRels[k].m_Target);
};

void  IncludeHigherInHierarchy(CHierarchyHolder* pHierarchyDoc, std::vector<std::string>& SemFets)
{
	for (int i = 0; i < SemFets.size(); i++)
		for (int k = 0; k < pHierarchyDoc->m_TransitiveRels.size(); k++)
			if ((SemFets[i] == pHierarchyDoc->m_TransitiveRels[k].m_Target)
				&& (find(SemFets.begin(), SemFets.end(), pHierarchyDoc->m_TransitiveRels[k].m_Source) == SemFets.end())
				)
				SemFets.push_back(pHierarchyDoc->m_TransitiveRels[k].m_Source);
};



const int MaxLevelId = 30;

// проверяет что все SF актанта с номером LeafId равны  ItemStr или 
// ниже по иерархии
bool SemFetActantIsEqualOrLower(CRossHolder* Ross, uint16_t Host, BYTE LeafId, BYTE BracketLeafId, const std::string& ItemStr, CHierarchyHolder* pHierarchyDoc)
{
	long ItemNo = Ross->GetRoss()->GetItemNoByItemStr(ItemStr, Ross->SemFetDomNo);
	assert(ItemNo != -1);
	long LastNo = Ross->GetRoss()->GetUnitEndPos(Host);
	bool Found = false;
	for (long i = Ross->GetRoss()->GetUnitStartPos(Host); i <= LastNo; i++)
		if ((Ross->GetRoss()->GetCortegeLeafId(i) == LeafId)
			&& (Ross->GetRoss()->GetCortegeBracketLeafId(i) == BracketLeafId)
			&& (Ross->GetRoss()->GetCortegeFieldNo(i) == Ross->SemFetFieldNo)
			)
		{

			Found = true;
			TCortege C = GetCortege(Ross->GetRoss(), i);
			if ((ItemNo != C.m_DomItemNos[0])
				&& !IsEqualOrHigherInHierarchy(Ross, ItemNo, Ross, C.m_DomItemNos[0], pHierarchyDoc)
				)
				return false;
		};
	return Found;
};


#include "StdGraph.h"
#include "GraphmatFile.h"
#include "GraphanDicts.h"
#include "../common/util_classes.h"



const char OborotDels[] = " \t";





// search the longest oborot, which starts from FirstGraLineNo 
size_t  CGraphmatFile :: FindOborotto (size_t FirstGraLineNo, size_t HB, short& ResultOborotNo, std::vector<uint16_t>& OborotIds) const
{
	ResultOborotNo = -1;
	
	if ( GetUnits()[FirstGraLineNo].IsGrouped() ) return FirstGraLineNo;
	
	size_t MaxEndLineNo =  FirstGraLineNo;
	typedef std::map<uint16_t, std::vector<uint16_t> > PMap;
	PMap::const_iterator it = m_pDicts->m_OborottosFirstWordIndex.find(OborotIds[FirstGraLineNo]);
	
	if (it == m_pDicts->m_OborottosFirstWordIndex.end())
		return FirstGraLineNo;

	const std::vector<uint16_t>& Oborots = it->second;
	size_t OborotsCount = Oborots.size();
	
	for (uint16_t OborotNo = 0; OborotNo < OborotsCount; OborotNo++)
	{
		const CGraphemOborot& O = m_pDicts->m_Oborottos[ Oborots[OborotNo] ];
		int OborotWordsCount = O.m_TokenIds.size();
		int CurrLine = FirstGraLineNo;
		int OborotWordNo = 0;
		for (;		
						OborotWordNo<OborotWordsCount
					&&	(CurrLine < HB)
					&&	( O.m_TokenIds[OborotWordNo] ==  OborotIds[CurrLine]) 
					&&	!GetUnits()[CurrLine].IsGrouped();
				OborotWordNo++
			)
			if (OborotWordNo + 1 < OborotWordsCount) // get the next graphematical line, if this iteration is  not last
				CurrLine = PSoft(CurrLine+1, HB);

		
		if (OborotWordNo == OborotWordsCount)
			if (CurrLine+1  > MaxEndLineNo)
			{
				 ResultOborotNo = Oborots[OborotNo];
				 MaxEndLineNo = CurrLine+1;
			};
	};
 
	return MaxEndLineNo;
};

void NormalizeOborotStr (const CGraphanDicts& C, std::string& S)
{
	
	RmlMakeUpper( S, C.m_Language );
	int len = S.length();
	int i = 0;
	std::string Norm;
	while (i < len )
	{
		std::string token;
		while	(	
						(i < len)
					&&	(		is_alpha((BYTE)S[i])
							||	isdigit((BYTE)S[i])
							||	((BYTE)S[i] == '-')
						)
				)
		{
			token += S[i];
			i++;
		};
		if (token.empty())
		{
			while ((i < len) && ispunct((BYTE)S[i]))
			{
				token += S[i];
				i++;
			};
			if (token.empty())
			{
				if (isspace((BYTE)S[i]) ) 
				{
					i++;
					continue;
				}
				else
				{
					throw CExpc ("Cannot parse oborot " + S);
				};
			};

		}
		Norm += token + " ";
	};

	Trim(Norm);
	S = Norm;
};

void  CGraphanDicts:: BuildOborot (const std::string& s, int OborotNo, bool bFixedFet)
{
	size_t i = s.find("(");
	if (i == s.npos) 
	{
		if (s.find("+") == s.npos) // if it is not a special dictionary entry 
		{
			CGraphemOborot O;
			O.m_bFixedFet = bFixedFet;
			O.m_UnitNo = OborotNo;
			O.m_UnitStr = s;
			NormalizeOborotStr(*this, O.m_UnitStr);
			if (find(m_Oborottos.begin(),  m_Oborottos.end(), O.m_UnitStr) == m_Oborottos.end())
				m_Oborottos.push_back(O);


			// приравниваем "в контакте с" и "в контакте со"
			std::string q = " " +  O.m_UnitStr;
			q = q.substr(q.length() - 2, 2);
			// если заканчивается на предлог "с", "в" или "к"
			if ( (q ==  _R(" К")) || (q ==  _R(" С")) || (q ==  _R(" В")) )
			{
				O.m_UnitStr += _R("О");
				if (find(m_Oborottos.begin(),  m_Oborottos.end(), O.m_UnitStr) == m_Oborottos.end())
					m_Oborottos.push_back(O);
			};
		};
	}
	else
	{
		size_t k = s.find (")");
		if (k == s.npos)
		{
			char Error[250];
			sprintf (Error, "Error in parenthesis  in oborot %s", s.c_str());
			ErrorMessage(Error);
		};
		size_t last_j = i;
        for (size_t j =i+1; j <= k; j++)
		{
          if  (    (j == k) 
			    || (s[j] == '|')) 
		  {
             std::string q;
			 if (i >0) q += s.substr(0,i);
			 q += " ";
			 q += s.substr(last_j+1, j-last_j-1);
			 q += " ";
		     if (k-1 < s.length()) q += s.substr(k+1);
			 BuildOborot(q, OborotNo, bFixedFet);
			 last_j = j;
		  };

		};

	};

};


void PrintOborottos (CGraphanDicts& C)
{
	FILE* fp = fopen ("oborots.txt", "w");
	for (size_t k=0; k < C.m_Oborottos.size(); k++)
	{
		const char* title =  C.m_Oborottos[k].m_UnitStr.c_str();
		fprintf (fp, "%s\n", title);

	};
	fclose (fp);
};


void BuildOborottosIndex (CGraphanDicts& C)
{
	StringVector AllUnsortedTokens;

	size_t OborotNo=0;

	for (; OborotNo < C.m_Oborottos.size(); OborotNo++)
	{
		const char* title =  C.m_Oborottos[OborotNo].m_UnitStr.c_str();
		int len = strlen(title);
		int i = 0;
		StringTokenizer  tok (C.m_Oborottos[OborotNo].m_UnitStr.c_str(), " ");
		while (tok())
		{
			std::string token = tok.val();
			StringVector::iterator it = find (AllUnsortedTokens.begin(),AllUnsortedTokens.end(), token);
			if	(		(it == AllUnsortedTokens.end())
					|| (*it == token)
				)
			{
				AllUnsortedTokens.push_back(token);
				it = AllUnsortedTokens.end();
				it--;
			};

			uint16_t id = it - AllUnsortedTokens.begin();
			C.m_Oborottos[OborotNo].m_TokenIds.push_back(id);

		};
	};

	C.m_OborotTokens = AllUnsortedTokens;
	sort(C.m_OborotTokens.begin(),C.m_OborotTokens.end());
	std::vector<uint16_t> T;
	T.resize( C.m_OborotTokens.size() );
	for (size_t i=0; i < AllUnsortedTokens.size(); i++)
		T[i] = lower_bound(C.m_OborotTokens.begin(),C.m_OborotTokens.end(), AllUnsortedTokens[i]) - C.m_OborotTokens.begin();

	for (OborotNo=0; OborotNo < C.m_Oborottos.size(); OborotNo++)
	{
		for (int k = 0; k<C.m_Oborottos[OborotNo].m_TokenIds.size(); k++)
			C.m_Oborottos[OborotNo].m_TokenIds[k] = T[ C.m_Oborottos[OborotNo].m_TokenIds[k] ];

		assert (!C.m_Oborottos[OborotNo].m_TokenIds.empty());
		C.m_OborottosFirstWordIndex[ C.m_Oborottos[OborotNo].m_TokenIds[0] ].push_back((uint16_t)OborotNo);
	};

};


void  CGraphanDicts :: BuildOborottos ()
{
	std::string OborotStr;

	try { 

		size_t count = GetOborDic()->GetUnitsSize();

		for (long i=0; i<count; i++)
			if (GetOborDic()->IsEmptyArticle(i) == false)
			{

				bool HasFixedFet = false;
				size_t k = GetOborDic()->GetUnitStartPos(i);
				for (; k <= GetOborDic()->GetUnitEndPos(i); k++)
					if (GetOborDic()->GetCortegeFieldNo(k) == GetOborDic()->GetFieldNoByFieldStr("RESTR"))
						if (GetOborDic()->GetCortegeItem(k,0) != -1)
					{
						std::string G = (const char*)GetOborDic()->GetDomItemStr(GetOborDic()->GetCortegeItem(k,0));
						if (strcmp(G.c_str(),"fixed") == 0)
						HasFixedFet = true;
					};

				for (k=GetOborDic()->GetUnitStartPos(i); k <= GetOborDic()->GetUnitEndPos(i); k++)
					if (GetOborDic()->GetCortegeFieldNo(k) == GetOborDic()->GetFieldNoByFieldStr("CONTENT"))
					{
						std::string q = GetOborDic()->GetDomItemStr(GetOborDic()->GetCortegeItem(k,0));
						BuildOborot(q, i, HasFixedFet);
					}
			};

	}
	catch (...)
	{
		ErrorMessage(Format("Error in oborot \"%s\"", OborotStr.c_str()) );

	};

	BuildOborottosIndex(*this);
};

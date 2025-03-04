// ==========  This file is under  LGPL, the GNU Lesser General Public Licence
// ==========  Dialing Lemmatizer (www.aot.ru)
// ==========  Copyright by Alexey Sokirko
#include "StdMorph.h"
#include "Lemmatizers.h"
#include "../common/utilit.h"
#include "Paradigm.h"
#include "../common/rus_numerals.h"

CLemmatizer::CLemmatizer(MorphLanguageEnum Language) : CMorphDict(Language), m_Predict(Language)
{	
	m_bLoaded = false;
	m_bUseStatistic = false;
	m_bMaximalPrediction = false;
	m_bAllowRussianJo = false;
	InitAutomat( new CMorphAutomat(Language, MorphAnnotChar) );
};

CLemmatizer::~CLemmatizer()
{
};


std::string CLemmatizer::GetPath()  const
{
	std::string RegStr = GetRegistryString();
	//fprintf (stderr,_R("1"));
	std::string load_path = ::GetRegistryString( RegStr );
	//fprintf (stderr,"2");
	if (	(load_path.length() > 0)	
		&&	(load_path[load_path.length() - 1] != '\\')
		&&	(load_path[load_path.length() - 1] != '/')
		)
		load_path += "/";

	return load_path;
};;


bool CLemmatizer::CheckABC(const std::string& WordForm) const
{
	return m_pFormAutomat->CheckABCWithoutAnnotator(WordForm);
};

bool CLemmatizer::CreateParadigmFromID(uint32_t id, CFormInfo& Result) const 
{
	Result.AttachLemmatizer(this);
	return Result.SetParadigmId(id);
}

bool CLemmatizer::IsHyphenPostfix(const std::string& Postfix) const
{
	return m_HyphenPostfixes.find(Postfix) != m_HyphenPostfixes.end();
};

bool CLemmatizer::IsHyphenPrefix(const std::string& Prefix) const
{
	return m_HyphenPrefixes.find(Prefix) != m_HyphenPrefixes.end();
};


const CStatistic& CLemmatizer::GetStatistic() const 
{	
	return m_Statistic;	
}



bool CLemmatizer::IsPrefix(const std::string& Prefix) const
{
	return m_PrefixesSet.find(Prefix) != m_PrefixesSet.end();

};

//   CLemmatizer::LemmatizeWord should return true if 
// the word was found in the dictionary, if it was predicted, then it returns false
bool CLemmatizer::LemmatizeWord(std::string& InputWordStr, const bool cap, const bool predict, std::vector<CAutomAnnotationInner>& results, bool bGetLemmaInfos) const
{
	
	RmlMakeUpper (InputWordStr, GetLanguage());

	size_t WordOffset = 0;
	

	m_pFormAutomat->GetInnerMorphInfos(InputWordStr, 0, results);

	bool bResult = !results.empty();

	if (results.empty())
	{
		if (predict)
		{
			PredictBySuffix(InputWordStr, WordOffset, 4, results); // the length of the minal suffix is 4 


			if (InputWordStr[WordOffset-1] != '-') //  and there is no hyphen
			{
				size_t  KnownPostfixLen = InputWordStr.length()-WordOffset;
				size_t  UnknownPrefixLen = WordOffset;
				if (KnownPostfixLen < 6)// if  the known part is too short
					//if	(UnknownPrefixLen > 5)// no prediction if unknown prefix is more than 5
					{
						if (!IsPrefix(InputWordStr.substr(0, UnknownPrefixLen)))
							results.clear();
					};
			};

			// отменяем предсказание по местоимениям, например _R("Семыкиным")
			for (size_t i=0; i<results.size(); i++)
				if (m_NPSs[results[i].m_ModelNo] == UnknownPartOfSpeech)
				{
					results.clear();
					break;
				};

		};
	};

	if (!results.empty())
	{
		if (bGetLemmaInfos)
			GetLemmaInfos(InputWordStr, WordOffset, results);
	}
	else
		if (predict)
		{
			PredictByDataBase(InputWordStr, results,cap);
			for (int i =results.size()-1; i>=0; i--)
			{
					const CAutomAnnotationInner& A = results[i];
					const CLemmaInfo& I = m_LemmaInfos[A.m_LemmaInfoNo].m_LemmaInfo;
					const CFlexiaModel& M = m_FlexiaModels[A.m_ModelNo];
					const CMorphForm& F = M.m_Flexia[A.m_ItemNo];
					if ( F.m_FlexiaStr.length() >= InputWordStr.length() )
					{
						results.erase(results.begin() + i);
					}
			}
		};

	return bResult;
}






void CLemmatizer::AssignWeightIfNeed(std::vector<CAutomAnnotationInner>& FindResults) const
{

	for (size_t i = 0; i < FindResults.size(); i++)
	{
		CAutomAnnotationInner& A = FindResults[i];
		if (m_bUseStatistic)
			A.m_nWeight = m_Statistic.get_HomoWeight(A.GetParadigmId(), A.m_ItemNo);
		else
			A.m_nWeight = 0;
	}

}

void CLemmatizer::GetAllAncodesQuick(const BYTE* WordForm, bool capital, BYTE* OutBuffer, bool bUsePrediction) const
{
	*OutBuffer = 0;
	std::string InputWordStr = (const char*)WordForm;
	FilterSrc(InputWordStr);
	std::vector<CAutomAnnotationInner>	FindResults;
	LemmatizeWord(InputWordStr, capital, bUsePrediction, FindResults, true);
	size_t Count = FindResults.size();
	for (uint32_t i=0; i < Count; i++)
	{
		const CAutomAnnotationInner& A = FindResults[i];
		const CFlexiaModel& M = m_FlexiaModels[A.m_ModelNo];
		const CLemmaInfo& I = m_LemmaInfos[A.m_LemmaInfoNo].m_LemmaInfo;
		// add common  ancode
		if (I.m_CommonAncode[0] != 0)
		{
			size_t l = strlen((char*)OutBuffer);
			OutBuffer[l] = I.m_CommonAncode[0];
			OutBuffer[l+1] = I.m_CommonAncode[1];
			OutBuffer[l+2] = 0;
		}
		else
			strcat((char*)OutBuffer, "??");
		strcat((char*)OutBuffer, M.m_Flexia[A.m_ItemNo].m_Gramcode.c_str());
		strcat((char*)OutBuffer, ";");
		
		
	};
	
}


bool CLemmatizer::GetAllAncodesAndLemmasQuick(std::string& InputWordStr, bool capital, char* OutBuffer, size_t MaxBufferSize, bool bUsePrediction) const
{
	FilterSrc(InputWordStr);

	std::vector<CAutomAnnotationInner>	FindResults;

	bool bFound = LemmatizeWord(InputWordStr, capital, bUsePrediction, FindResults, false);

	size_t Count = FindResults.size();
	size_t  OutLen = 0;
	for (uint32_t i=0; i < Count; i++)
	{
		const CAutomAnnotationInner& A = FindResults[i];
		const CFlexiaModel& M = m_FlexiaModels[A.m_ModelNo];
		const CMorphForm& F = M.m_Flexia[A.m_ItemNo];
		size_t PrefixLen = F.m_PrefixStr.length();
		size_t BaseStart  = 0;
		if	(		bFound 
				||	!strncmp(InputWordStr.c_str(), F.m_PrefixStr.c_str(), PrefixLen)
			)
			BaseStart = PrefixLen;
		int BaseLen  = InputWordStr.length() - F.m_FlexiaStr.length() - BaseStart;
		if (BaseLen < 0) BaseLen = InputWordStr.length();
		size_t GramCodeLen = M.m_Flexia[A.m_ItemNo].m_Gramcode.length();
		size_t FlexiaLength = M.m_Flexia[0].m_FlexiaStr.length();
		if (BaseLen+FlexiaLength+3+GramCodeLen > MaxBufferSize-OutLen) return false; 

		strncpy(OutBuffer+OutLen, InputWordStr.c_str()+BaseStart, BaseLen);
		OutLen += BaseLen;

		strncpy(OutBuffer+OutLen, M.m_Flexia[0].m_FlexiaStr.c_str(), FlexiaLength);
		OutLen += FlexiaLength;

		OutBuffer[OutLen] = ' ';
		OutLen++;

		strncpy(OutBuffer+OutLen, M.m_Flexia[A.m_ItemNo].m_Gramcode.c_str(), GramCodeLen);
		OutLen += GramCodeLen+1;
		OutBuffer[OutLen-1] = 	'#';

	};
	OutBuffer[OutLen] = 0;
	return true;
	
}


//////////////////////////////////////////////////////////////////////////////

void CLemmatizer::ReadOptions(std::string FileName)
{
	std::string Options;
	LoadFileToString(FileName, Options);
	StringTokenizer lines(Options.c_str(), "\r\n");
	while (lines())
	{
		std::string line = lines.val();
		Trim(line);
		if (line.empty()) continue;
		if (line == "AllowRussianJo")
			m_bAllowRussianJo = true;
	};
};


bool CLemmatizer::LoadDictionariesRegistry(std::string& strError)
{
	std::string load_path;
	try
	{
		load_path = GetPath();
	}
	catch (...)
	{
		strError = "Cannot read $RML/Bin/rml.ini; $RML="+GetRmlVariable();
		return false;
	}
	try{
		m_bLoaded = Load(load_path+MORPH_MAIN_FILES);
		if (!m_bLoaded) 
		{
			strError = "Cannot load " + load_path+MORPH_MAIN_FILES;
			return false;
		}

		// implicity load homonyms statistic for literature
		m_Statistic.Load(load_path + "l");
		m_bUseStatistic = true;
		m_Predict.Load(load_path + PREDICT_BIN_PATH);

		//  building frequences of flexia models
		{
			m_Predict.m_ModelFreq.resize(m_FlexiaModels.size(), 0);
			const size_t count = m_LemmaInfos.size(); 
			for(size_t i=0; i < count; i++)
				m_Predict.m_ModelFreq[m_LemmaInfos[i].m_LemmaInfo.m_FlexiaModelNo]++;
		};



		ReadOptions(load_path + OPTIONS_FILE);
		m_PrefixesSet.clear();
		m_PrefixesSet.insert(m_Prefixes.begin(), m_Prefixes.end() );

		return m_bLoaded;
	}
	catch(CExpc& s)
	{
		strError = s.m_strCause+"; RML="+GetRmlVariable();
		return false;
	}
	catch(...)
	{
		strError = "General exception; RML="+GetRmlVariable();
		return false;
	}
}

bool IsFound(const std::vector<CFormInfo> & results)
{
    return !results.empty() && results[0].m_bFound;

};

void CreateDecartProduction (const std::vector<CFormInfo>& results1, const std::vector<CFormInfo>& results2, std::vector<CFormInfo>& results)
{
	results.clear();
	for (size_t i=0; i<results1.size(); i++)
		for (size_t k=0; k<results2.size(); k++)
		{
            CFormInfo F = results2[k];
            F.SetUserPrefix(results1[i].GetWordForm(0)+"-");
			results.push_back(F);
		};

};



bool CLemmatizer::CreateParadigmCollection(bool bNorm, std::string& InputWordStr, bool capital, bool bUsePrediction, std::vector<CFormInfo>& Result) const
{
    Result.clear();
	FilterSrc(InputWordStr);
	std::vector<CAutomAnnotationInner>	FindResults;
	bool bFound = LemmatizeWord(InputWordStr, capital, bUsePrediction, FindResults, true);
		
	AssignWeightIfNeed(FindResults);

	for (size_t i = 0; i < FindResults.size(); i++)
	{
		const CAutomAnnotationInner& A = FindResults[i];
		// if bNorm, then  ignore words which are not lemma
		if (   bNorm && (A.m_ItemNo!=0)) continue;
		
		CFormInfo P;
		P.Create(this, A, InputWordStr, bFound);

		Result.push_back(P);
	}

	if (!IsFound(Result)) // not found or predicted
	{
		// if the word was not found in the dictionary 
		// and the word contains a hyphen 
		// then we should try to predict each parts of the hyphened word separately
        std::vector<CFormInfo> results1, results2;

		int hyph = InputWordStr.find("-");
		bool gennum = false;
		int pos = -1;
		if( GetLanguage() == morphRussian && InputWordStr.length() > 12 && hyph == std::string::npos)
		for(int n = 0; n < NumeralToNumberCount; n++)
		{
			if( !NumeralToNumber[n].m_bNoun ) continue;
			int len = NumeralToNumber[n].m_Ordinal.length();
			if(NumeralToNumber[n].m_Ordinal[0]!=0 && InputWordStr.length()>len+1
				&& (pos = InputWordStr.substr(InputWordStr.length() - len - 1).rfind(std::string(NumeralToNumber[n].m_Ordinal).substr(0, len - 2))) != std::string::npos)
			{
				std::string s = InputWordStr.substr(InputWordStr.length() - len - 1 + pos);
				CreateParadigmCollection(false, s, capital, false, results1 );
				if ( results1.size()>0 )
				{
					Result = results1;
					Result[0].m_InputWordBase = InputWordStr.substr(0, InputWordStr.length() - len - 1 + pos) 
						+ Result[0].m_InputWordBase;
					Result[0].SetUserUnknown();
				}
				break;
			}
		}
		if (hyph != std::string::npos)
		{
			// try to lemmatize each parts without predictions
			std::string first_part = InputWordStr.substr(0, hyph);
			std::string second_part = InputWordStr.substr(gennum ? hyph : hyph+1);
			CreateParadigmCollection(false, first_part, capital, false, results1 );

			/*
			 if the first part is equal to the second part  or the second part is an unimportant: Russian postfix
			 then we should use only the first part 
			*/
			if	(			(first_part == second_part)
					||		IsHyphenPostfix(second_part)
				)
				Result = results1;

            else
            if (IsHyphenPrefix(first_part))
            {
                CreateParadigmCollection(false, second_part, capital,  false, results2 );
                if (IsFound(results2))
                {
                    Result = results2;
                    for (int i=0; i < Result.size(); i++)
                    {
                        Result[i].SetUserPrefix(first_part+"-");
                        Result[i].SetUserUnknown();
                        
                    }

                }

            }
			else
			{
				CreateParadigmCollection(false,second_part, false, false, results2 );
                if (IsFound(results1) && IsFound(results2) && first_part.length()>2  && second_part.length()>2)
                {
					// if both words were found in the dictionary
					// then we should create a decart production
					CreateDecartProduction(results1, results2, Result);
                    for (int i=0; i < Result.size(); i++)
                        Result[i].SetUserUnknown();
                    
                    
                }
			}
		};
	};


	return true;
}


bool CLemmatizer::LoadStatisticRegistry(SubjectEnum subj)
{
	try
	{
		std::string load_path = GetPath();
		std::string prefix;
		switch (subj)
		{
		case subjFinance:
			prefix += "f";
			break;
		case subjComputer:
			prefix += "c";
			break;
		case subjLiterature:
			prefix += "l";
			break;
		default:
			return false;
		}
		m_Statistic.Load(load_path + prefix);
		return true;
	}
	catch(...)
	{
		return false;
	}
}

CAutomAnnotationInner  CLemmatizer::ConvertPredictTupleToAnnot(const CPredictTuple& input) const  
{
	CAutomAnnotationInner node;
	node.m_LemmaInfoNo = input.m_LemmaInfoNo;
	node.m_ModelNo = m_LemmaInfos[node.m_LemmaInfoNo].m_LemmaInfo.m_FlexiaModelNo;
	node.m_nWeight = 0;
	node.m_PrefixNo = 0;
	node.m_ItemNo = input.m_ItemNo;
	return node;
};


bool CLemmatizer::CheckAbbreviation(std::string InputWordStr,std::vector<CAutomAnnotationInner>& FindResults, bool is_cap) const
{
	for(size_t i=0; i <InputWordStr.length(); i++)
		if (!is_upper_consonant((BYTE)InputWordStr[i], GetLanguage()))
			return false;

	std::vector<CPredictTuple> res;
	m_Predict.Find(m_pFormAutomat->GetCriticalNounLetterPack(),res); 
	FindResults.push_back(ConvertPredictTupleToAnnot(res[0]));
	return true;
};

void CLemmatizer::PredictByDataBase(std::string InputWordStr,  std::vector<CAutomAnnotationInner>& FindResults,bool is_cap) const  
{

	std::vector<CPredictTuple> res;
	if (CheckAbbreviation(InputWordStr, FindResults, is_cap))
		return;

	if (CheckABC(InputWordStr)) // if the ABC is wrong this prediction yuilds to many variants
	{		
		reverse(InputWordStr.begin(),InputWordStr.end());
		m_Predict.Find(InputWordStr,res);
	}

	std::vector<int> has_nps(32, -1); // assume not more than 32 different pos
	for( int j=0; j<res.size(); j++ )
	{
		BYTE PartOfSpeechNo = res[j].m_PartOfSpeechNo;
		if	(		!m_bMaximalPrediction 
				&& (has_nps[PartOfSpeechNo] != -1) 
			)
		{
			int old_freq = m_Predict.m_ModelFreq[FindResults[has_nps[PartOfSpeechNo]].m_ModelNo];
			int new_freq = m_Predict.m_ModelFreq[m_LemmaInfos[res[j].m_LemmaInfoNo].m_LemmaInfo.m_FlexiaModelNo];
			if (old_freq < new_freq)
				FindResults[has_nps[PartOfSpeechNo]] = ConvertPredictTupleToAnnot(res[j]);
			
			continue;
		};

		has_nps[PartOfSpeechNo] = FindResults.size();

		FindResults.push_back(ConvertPredictTupleToAnnot(res[j]));
	}

	if	(		(has_nps[0] == -1) // no noun
			||	( is_cap && (GetLanguage() != morphGerman)) // or can be a proper noun (except German, where all nouns are written uppercase)
		)
	{
		m_Predict.Find(m_pFormAutomat->GetCriticalNounLetterPack(),res); 
		FindResults.push_back(ConvertPredictTupleToAnnot(res.back()));
	};

}


bool CLemmatizer::ProcessHyphenWords(CGraphmatFile* piGraphmatFile) const 
{
	try
	{
		
		size_t LinesCount = piGraphmatFile->GetTokensCount();
	

		for (int LineNo = 1; LineNo+1 < LinesCount; LineNo++)
		{
			if	(		piGraphmatFile->HasDescr(LineNo, OHyp)
					&&	(GetLanguage() == piGraphmatFile->GetTokenLanguage(LineNo-1)

					// and if no single space was deleted between the first word and the hyphen
					&&	!piGraphmatFile->GetUnits()[LineNo-1].HasSingleSpaceAfter()

					&&	!piGraphmatFile->HasDescr(LineNo-1, OSentEnd)
					&&	!piGraphmatFile->HasDescr(LineNo, OSentEnd)

					// it is not possible to create a hyphen word when this part of the text has an intersection 
					// with an oborot which is marked as "fixed" in the dictionary 
					&&	!piGraphmatFile->StartsFixedOborot(LineNo))
					&&	!piGraphmatFile->StartsFixedOborot(LineNo-1)
				)
			{
				//  creating a concatenation if it possible
				size_t NextWord = piGraphmatFile->PSoft(LineNo+1, LinesCount);
				if (NextWord == LinesCount) continue;

				// it is not possible to create a hyphen word  when this part of the text has an intersection 
				// with an oborot which is marked as "fixed" in the dictionary 
				if (piGraphmatFile->StartsFixedOborot(NextWord)) continue;
				
				if (GetLanguage() != piGraphmatFile->GetTokenLanguage(NextWord)) continue;
				std::string HyphenWord = piGraphmatFile->GetToken(LineNo-1)+"-"+piGraphmatFile->GetToken(NextWord);


				std::vector<CAutomAnnotationInner>	FindResults;
				if (LemmatizeWord(HyphenWord, !piGraphmatFile->HasDescr(LineNo-1,OLw), false,FindResults, false))
				{
					// uniting words LineNo-1, LineNo,  and NextWord
					piGraphmatFile->MakeOneWord(LineNo-1, NextWord+1);

					//  update LinesCount
					LinesCount = piGraphmatFile->GetTokensCount();
				};
			};

		};

	}
	catch (...)
	{
		return false;
	}
	return true;

};



CLemmatizerRussian::CLemmatizerRussian() : CLemmatizer(morphRussian)
{
	m_Registry = "Software\\Dialing\\Lemmatizer\\Russian\\DictPath";
	m_HyphenPostfixes.insert(_R("КА"));
	m_HyphenPostfixes.insert(_R("ТО"));
	m_HyphenPostfixes.insert(_R("С"));

    m_HyphenPrefixes.insert(_R("ПОЛУ"));
    m_HyphenPrefixes.insert(_R("ПОЛ"));
    m_HyphenPrefixes.insert(_R("ВИЦЕ"));
    m_HyphenPrefixes.insert(_R("МИНИ"));
    m_HyphenPrefixes.insert(_R("КИК"));
};


void CLemmatizerRussian::FilterSrc(std::string& src) const	
{
	if (!m_bAllowRussianJo)
		ConvertJO2Je(src); 

	// переводим ' в _R("ъ"), например, "об'явление" -> _R("объявление")
	size_t len = src.length();
	for (size_t i=0; i<len; i++)
		if (src[i] == '\'')
			src[i] = _R("ъ")[0];
};

CLemmatizerEnglish:: CLemmatizerEnglish() : CLemmatizer(morphEnglish)
{
	m_Registry = "Software\\Dialing\\Lemmatizer\\English\\DictPath";
};

void CLemmatizerEnglish:: FilterSrc(std::string& src) const
{
};

CLemmatizerGerman:: CLemmatizerGerman() : CLemmatizer(morphGerman)
{
	m_Registry = "Software\\Dialing\\Lemmatizer\\German\\DictPath";
};

void CLemmatizerGerman:: FilterSrc(std::string& src) const
{
};



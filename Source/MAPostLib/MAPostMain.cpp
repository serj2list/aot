// ==========  This file is under  LGPL, the GNU Lesser General Public Licence
// ==========  Dialing Posmorphological Module (www.aot.ru)
// ==========  Copyright by Dmitry Pankratov, Alexey Sokirko (1999-2002)

#include "MAPostMain.h"
#include "../common/PlmLine.h"
#include "../common/utilit.h"
#include "../TrigramLib/TrigramModel.h"
#include "../AgramtabLib/RusGramTab.h"

void CMAPost::log(std::string s)
{
	if (m_LogFileName == "")  return;
	try {
		FILE* fp = fopen(m_LogFileName.c_str(), "a");
		if (!fp)
		{
			m_LogFileName = "";
			return;
		};
		fprintf(fp, "%s\n", s.c_str());
		fclose(fp);
	}
	catch (...) {
	};

}


void CMAPost::RunRules()
{
	try
	{
		Rule_TwoPredicates();

		if (m_bUseTrigrams)
			FilterPostMorphWords();

		log("Rule_FilterProperName");
		Rule_FilterProperName();

		log("Rule_AdverbFromAdjectives");
		Rule_AdverbFromAdjectives();

		log("Odnobuk");
		Odnobuk();


		if (m_bCanChangeInputText)
		{
			log("Cifrdef");
			Cifrdef();

			log("ILeDefLe");
			ILeDefLe();

		}

		log("ParticipleAndVerbInOneForm");
		ParticipleAndVerbInOneForm();

		log("PronounP_Pronoun_Homonymy");
		PronounP_Pronoun_Homonymy();


		if (m_bCanChangeInputText)
		{
			log("FixedCollocations");
			FixedCollocations();
		};

		log("CorrectOborots");
		CorrectOborots();

		log("SemiAdjectives");
		SemiAdjectives();

		log("Interjections");
		Rule_Interjections();

		log("SemiNouns");
		SemiNouns();

		log("Rule_UZHE");
		Rule_UZHE();

		log("Rule_Ideclinable");
		Rule_Ideclinable();

		log("Rule_DeadPlurals");
		Rule_DeadPlurals();

		log("Rule_RelationalAdjective");
		Rule_RelationalAdjective();

		log("Rule_Fio");
		Rule_Fio();


		if (m_bCanChangeInputText)
		{
			log("Rule_QuoteMarks");
			Rule_QuoteMarks();
		};

		log("Rule_ILE");
		Rule_ILE();

		if (m_bCanChangeInputText)
		{
			log("Rule_KAK_MOZHNO");
			Rule_KAK_MOZHNO();

			log("Rule_Redublication");
			Rule_Redublication();

			log("Rule_CHTO_ZA");
			Rule_CHTO_ZA();

			log("Rule_VOT_UZHE");
			Rule_VOT_UZHE();

		};

		log("Rule_UnknownNames");
		Rule_UnknownNames();

		log("Rule_SOROK");
		Rule_SOROK();

		Rule_ExpandIndeclinableGramcodes();

		Rule_Abbreviation();

		Rule_ChangePatronymicLemmas();

		OtherRules();
	}
	catch (...)
	{
		ErrorMessage("Mapost", "Mapost has crushed!");
		return;
	}
}



bool CMAPost::LoadWords(const CPlmLineCollection* piInTextItems)
{
	long i = 0;
	try
	{
		m_Words.clear();
		int CurrOborotNo = -1;

		for (; i < piInTextItems->m_PlmItems.size(); i++)
		{
			const char* strPlmLine = piInTextItems->m_PlmItems[i].c_str();
			if (!CheckIfHomonymPlmLine(strPlmLine))
			{
				//  reading the first homonym and word's  properties 
				CPostLemWord Word(m_pRusGramTab);
				if (!Word.ProcessPlmLineForTheFirstHomonym(strPlmLine, morphRussian, CurrOborotNo))
					return false;

				Word.m_bHasSpaceBefore = m_Words.empty()
					|| m_Words.back().m_GraphematicalUnitOffset + m_Words.back().m_strWord.length() < Word.m_GraphematicalUnitOffset
					|| m_Words.back().m_bSpace;
				m_Words.push_back(Word);


			}
			else
				if (!m_Words.empty()) // // первая строка иногда содержит всякую информацию паро документ
				{
					if (!m_Words.back().AddNextHomonym(strPlmLine))
						return false;
				}

		}
		return true;
	}
	catch (...)
	{
		ErrorMessage("MAPOST", Format("Cannot read line %i (\"%s\") from morphology", i, piInTextItems->m_PlmItems[i].c_str()));
		return false;
	}
}

bool CMAPost::ProcessData(const CPlmLineCollection* piInTextItems)
{
	try
	{
		LoadWords(piInTextItems);

		RunRules();

		m_ResultLemWords.clear();
		for (std::list<CPostLemWord>::const_iterator it = m_Words.begin(); it != m_Words.end(); it++)
			for (size_t i = 0; i < it->GetHomonymsCount(); i++)
			{
				if (m_bHumanFriendlyOutput)
					m_ResultLemWords.push_back(it->GetDebugString(it->GetHomonym(i), i == 0));
				else
					m_ResultLemWords.push_back(it->GetPlmStr(it->GetHomonym(i), i == 0));
			}

		m_Words.clear();
		return true;

	}
	catch (...)
	{
		return false;
	}
}



//================================================
//======          ПРАВИЛА     ===================
//================================================





/*
 если после однобуквенного слова в верхнем регистре стоит точка,
 будем считать этот инициалом,  а не союзом.
*/

void CMAPost::Odnobuk()
{
	for (CLineIter it = m_Words.begin(); it != m_Words.end(); it++)
	{
		// однобуквенное слово,  след.- точка
		CPostLemWord& W = *it;
		if (!W.HasDes(ORLE)) continue;
		if (W.m_strWord.length() != 1) continue;
		if (!W.m_bFirstUpperAlpha) continue;
		if (W.HasOborot1() != W.HasOborot2()) continue;// "т.е." - это оборот, его обрабатывать не надо


		const part_of_speech_mask_t poses = (1 << PREP) | (1 << CONJ) | (1 << INTERJ) | (1 << PARTICLE);
		// данное слово может являться  только предлогом, союзом, междометием или частицей
		if (W.GetPoses() & ~poses) continue;//?? "по Б.Татарской улице"
		CLineIter next_it = it;
		next_it++;
		if (next_it == m_Words.end())continue;
		if (next_it->m_strWord == ".")
		{
			W.DeleteOborotMarks();
			W.DeleteAllHomonyms();
			CHomonym* pH = W.AddNewHomonym();
			pH->m_strLemma = W.m_strUpperWord;
			pH->SetMorphUnknown();
			pH->SetGramCodes(m_DURNOVOGramCode);
			pH->InitAncodePattern();
		}
	}
}

/*
 1. Если после слова нет знака препинания, тогда это слово не может междометием
 2. Если перед словами "а" и "но" нет знака препиания и эти слов не начинают предложение,
	тогда  они не могут быть союзами,


*/

void CMAPost::Rule_Interjections()
{
	bool bAfterPunctOrSentEnd = true;
	for (CLineIter it = m_Words.begin(); it != m_Words.end(); it++)
		if (!it->m_bSpace)
		{
			CPostLemWord& W = *it;
			if (W.HasDes(ORLE))
			{
				if (W.m_strUpperWord == _R("А") || W.m_strUpperWord == _R("НО"))
				{
					if (!bAfterPunctOrSentEnd)
						W.KillHomonymOfPartOfSpeech(CONJ);
					else
						W.KillHomonymOfPartOfSpeech(PARTICLE);
				}

				if (W.HasPos(INTERJ))
				{
					CLineIter next_it = NextNotSpace(it);
					if (next_it == m_Words.end() || !next_it->HasDes(OPun))
						W.KillHomonymOfPartOfSpeech(INTERJ);
				}
			}
			bAfterPunctOrSentEnd = W.HasDes(OPun) || W.HasDes(OSentEnd);;
		};
};



//
/*
  процедура  ищет  последовательность  LLE HYP Х, где Х - это ЛЕ, у которой
  есть омонимы на прилагательное или существительно, если нашлось. тогда LLE и HYP
  ставятся в начало леммы и словоформы Х, а строки  LLE и HYP уничтожаются.
  Работает на примерах:
	 ftp-сервер
	 ftp-серверный
*/
void CMAPost::ILeDefLe()
{
	for (CLineIter it = m_Words.begin(); it != m_Words.end(); it++)
	{
		CPostLemWord& W = *it;
		if (W.m_strWord.length() < 3) continue;
		int hyp = W.m_strWord.find("-");
		if (hyp == std::string::npos) continue;
		// первая часть - латиница, второая - русская
		if (!is_english_alpha((BYTE)W.m_strWord[0])
			|| is_russian_alpha((BYTE)W.m_strWord[0]) //   is_english_alpha и is_russian_alpha пересекаются
			|| !is_russian_alpha((BYTE)W.m_strWord[W.m_strWord.length() - 1])) continue;
		if (W.IsInOborot()) continue;

		if (W.LemmatizeForm(W.m_strWord.substr(hyp + 1), m_pRusLemmatizer))
			for (int i = 0; i < W.GetHomonymsCount(); i++)
			{
				CHomonym* pH = W.GetHomonym(i);
				pH->m_lPradigmID = -1;
				pH->m_LemSign = '-';
				if (pH->m_CommonGramCode.empty())
					pH->m_CommonGramCode = "??";
				pH->m_strLemma.insert(0, W.m_strUpperWord.substr(0, hyp + 1));
			}
	}
}




// идем по всем словам
// если найдена словоформа, которой одновременно приписано личная форма глагола 
// и причастие, то разделяем эту словоформу на два омонома
// Например, "выдвинут  + ВЫДВИНУТЬ фжцж"
// фж Г св,пе,дст,буд,3л,мн
// цж Г св,пе,прч,прш,стр,ед,мр,кр
// после работы алгоритма получаем 
// выдвинут  + ВЫДВИНУТЬ фж
//   выдвинут  + ВЫДВИНУТЬ цж

void CMAPost::ParticipleAndVerbInOneForm()
{
	for (CLineIter it = m_Words.begin(); it != m_Words.end(); it++)
	{
		CPostLemWord& W = *it;

		if (!W.HasPos(PARTICIPLE_SHORT) || !W.HasPos(VERB)) continue;

		for (int HomNo = 0; HomNo < W.GetHomonymsCount(); HomNo++)
		{
			CHomonym* pH = W.GetHomonym(HomNo);
			const std::string& GramCodes = pH->GetGramCodes();
			std::string VerbGramCodes;
			std::string PartGramCodes;
			if (GramCodes == "??") continue;
			for (long i = 0; i < GramCodes.length(); i += 2)
			{
				std::string gram = GramCodes.substr(i, 2);
				BYTE POS = m_pRusGramTab->GetPartOfSpeech(gram.c_str());
				if (POS == PARTICIPLE_SHORT)
					PartGramCodes += gram;
				else
					if (POS == VERB)
						VerbGramCodes += gram;
			};
			if (VerbGramCodes.empty() || PartGramCodes.empty()) continue;
			pH->SetGramCodes(VerbGramCodes);
			pH->InitAncodePattern();
			CHomonym NewHom = *pH;

			CHomonym* pNewHom = W.AddNewHomonym();
			*pNewHom = NewHom;
			pNewHom->SetGramCodes(PartGramCodes);
			pNewHom->InitAncodePattern();
		}
	};
};

/*
 идем по всем словам, пусть Х - текущая  русская словоформа =/= "это", "то", "их", "его", "ее"
 Пусть Х одновременно приписано два омонима МС-П
 и МС (например, "свое", "каждый"). Проверяем, стоит ли после этой словоформы
 неомонимичное существительное,  причастие, прилагательное, или местоим. прил, согласованное с этим по роду числу и падежу.
 Если стоит, тогда удаляем омоноим МС.
 Например,
   +свои люди
   +всякий  человек
   -если я найду, то облако взлетит.
   -Это облако.
   -я считал его/ее великим учителем
   -я считал их идиотами.
*/
void CMAPost::PronounP_Pronoun_Homonymy()
{
	for (CLineIter it = m_Words.begin(); it != m_Words.end(); it++)
	{
		CPostLemWord& W = *it;
		if (!W.HasPos(PRONOUN_P) || !W.HasPos(PRONOUN)) continue;

		if ((W.m_strUpperWord == _R("ЭТО"))
			|| (W.m_strUpperWord == _R("ТО"))
			|| (W.m_strUpperWord == _R("ИХ"))
			|| (W.m_strUpperWord == _R("ЕГО"))
			|| (W.m_strUpperWord == _R("ЕЕ"))
			|| (W.m_strUpperWord == _R("ЕЁ"))
			)
			continue;

		int HomNo = W.GetHomonymByPOS(PRONOUN_P);
		if (HomNo == -1)  continue;

		CLineIter next_it = NextNotSpace(it);
		if (next_it == m_Words.end()) continue;
		if (next_it->GetHomonymsCount() == 0)  continue;

		bool bAgreed = true;
		for (int i = 0; i < next_it->GetHomonymsCount(); i++)
		{
			const CHomonym* pNoun = next_it->GetHomonym(i);
			bAgreed = bAgreed && m_pRusGramTab->GleicheGenderNumberCase(pNoun->m_CommonGramCode.c_str(), pNoun->GetGramCodes().c_str(), W.GetHomonym(HomNo)->GetGramCodes().c_str());
		}
		if (bAgreed)
			W.KillHomonymOfPartOfSpeech(PRONOUN);

	};

};

// выдает по форме и части речи парадигму
bool CMAPost::HasParadigmOfFormAndPoses(std::string WordForm, part_of_speech_mask_t Poses) const
{
	std::vector<CFormInfo> Paradigms;
	m_pRusLemmatizer->CreateParadigmCollection(false, WordForm, false, false, Paradigms);

	for (long k = 0; k < Paradigms.size(); k++)
	{
		std::string AnCode = Paradigms[k].GetSrcAncode();
		BYTE POS = m_pRusGramTab->GetPartOfSpeech(AnCode.c_str());
		uint64_t Grams;
		m_pRusGramTab->GetGrammems(AnCode.c_str(), Grams);
		if (Poses & (1 << POS))
			return true;
	};

	return false;
}





/*
 функция ищет словосочетания по леммам, если нашла удаляет все строки, кроме главного слова
 в  главном слове лемму заменяет на интерфейсную строку CFixedColloc::m_InterfaceString
 например, оборот "кто бы то ни был" заменится на "кто?"
*/
void CMAPost::FixedCollocations()
{
	for (CLineIter it = m_Words.begin(); it != m_Words.end(); it = NextNotSpace(it))
	{
		for (long CollocNo = 0; CollocNo < m_FixedCollocs.size(); CollocNo++)
		{
			CLineIter main_it = m_Words.end();
			CHomonym* main_hom = 0;
			bool HasEndSent = false;
			long ItemNo = 0;
			const CFixedColloc& C = m_FixedCollocs[CollocNo];
			int OpenOborots = 0;;
			CLineIter curr_it = it;
			for (; ItemNo < C.m_Lemmas.size() && curr_it != m_Words.end(); ItemNo++, curr_it = NextNotSpace(curr_it))
			{
				CPostLemWord& W = *curr_it;
				/*
					Если найденное словосочетание пересекается с оборотом, но не включает его, и оборот не включает данное
					словосочетание(например,"что за счет его" оборот "за счет"), тогда  не будем строить это
				*/
				if (W.HasOborot1())     OpenOborots++;
				if (W.HasOborot2())     OpenOborots--;
				HasEndSent |= W.HasDes(OSentEnd);


				int i = 0;
				for (; i < W.GetHomonymsCount(); i++)
				{
					const CHomonym* pH = W.GetHomonym(i);

					if ((pH->m_strLemma == C.m_Lemmas[ItemNo].m_Lemma) // проверим лемму
						&& ((C.m_Lemmas[ItemNo].m_POS == UnknownPartOfSpeech) // проверим часть речи
							|| pH->HasPos(C.m_Lemmas[ItemNo].m_POS)
							)
						)
					{
						W.SetAllOtherHomsDel(i); // помечаем  найденный омоним
						break;
					}
				}

				if (i == W.GetHomonymsCount()) break; // эта лемма не совпала

				// сохраняем главную строку словосочетания, чтобы потом ее не удалять
				if (ItemNo == C.m_MainWordNo)
					main_it = curr_it;
			};

			if (OpenOborots == 0)
				if (ItemNo == C.m_Lemmas.size())
				{
					// нашли словосочетание.
					// it  указывает на первое слово словосочетания
					// curr_it указывает на первое слово, которое уже не входит в словосочетание

					if (HasEndSent)
						main_it->AddDes(OSentEnd);

					main_it->SafeDeleteMarkedHomonyms();
					assert(main_it->GetHomonymsCount() == 1);
					// меняем лемму в главное строке
					main_it->GetHomonym(0)->m_strLemma = C.m_InterfaceString;
					m_Words.erase(it, main_it);
					it = main_it;
					it++;
					m_Words.erase(it, curr_it);
					// выходим из цикла по словосочетаниям, поскольку нашли хотя бы одно словосочетание
					it = main_it;
					break;
				};
		}; // цикл по словосочетаниям

	}
};


void CMAPost::OtherRules()
{
	for (CLineIter it = m_Words.begin(); it != m_Words.end(); it++)
	{
		CPostLemWord& W = *it;
		CLineIter next_it = it;
		CLineIter prev_it = it;
		next_it++;
		if (it != m_Words.begin())
		{
			prev_it--;
			if (W.HasPos(VERB) && W.GetHomonymsCount() > 0 && (*prev_it).GetHomonymsCount() == 1 && (*prev_it).HasPos(PREP))
				for (int HomNo = 0; HomNo < W.GetHomonymsCount(); HomNo++)
					if (W.GetHomonym(HomNo)->HasPos(VERB))
						W.EraseHomonym(HomNo);
		}
		if (W.HasDes(ONumChar) && is_alpha((BYTE)W.m_strWord[0]) && W.GetPoses() == 0)
		{
			W.DeleteAllHomonyms();
			CHomonym* pNew = W.AddNewHomonym();
			pNew->SetMorphUnknown();
			pNew->m_strLemma = W.m_strUpperWord;
			pNew->m_CommonGramCode = _R("Фа");
			pNew->SetGramCodes(m_pRusGramTab->GetGramCodes(NOUN, rAllCases | rAllNumbers | rAllGenders, 0));//m_DURNOVOGramCode;//"Йшааабавагадаеажазаиайакаласгагбгвгггдгегжгзгигйгкглеаебевегедееежезеиейекелижизииийикил";
			pNew->InitAncodePattern();
			W.AddDes(is_alpha((BYTE)W.m_strWord[0], morphEnglish) ? OLLE : ORLE);
		}
	};
}

/*
 удаляем пометы EXPR1 и EXPR_NOxxx из графемат. помет, если для него  не было найдено
  EXPR2 через 20 слов
 */

void CMAPost::CorrectOborots()
{
	for (CLineIter it = m_Words.begin(); it != m_Words.end(); it++)
	{
		CPostLemWord& W = *it;
		if (W.HasOborot1())
		{
			int Count = 0;
			for (CLineIter next_it = it; next_it != m_Words.end(); next_it++)
			{
				if ((Count > 20)
					|| (next_it->HasOborot1() && it != next_it)
					)
				{
					W.DeleteOborotMarks();
					break;
				}
				if (next_it->HasOborot2())
					break;
				Count++;
			}
		}
	}
};

/*
 Правило проходит по всем словам, которые не были найдены в морфологии.
 Смотрит, не начинаются ли они с префикса "полу-",  если начинаются, и в морфологии есть
 такое же прилагательное  без приставки, тогда ставим лемму без приставки, а в графем.
 пометы заносим  графету "#ПОЛУ"
*/

void CMAPost::SemiAdjectives()
{
	for (CLineIter it = m_Words.begin(); it != m_Words.end(); it++)
	{
		CPostLemWord& W = *it;
		if (!W.HasDes(ORLE)) continue;
		if (!W.m_bPredicted) continue;
		if (W.m_strUpperWord.substr(0, 4) != _R("ПОЛУ")) continue;
		for (int i = 0; i < W.GetHomonymsCount(); i++)
			W.GetHomonym(i)->m_strLemma.erase(0, 4);
		// установка графематической пометы
		W.m_UnparsedGraphemDescriptorsStr += _R(" #ПОЛУ ");
	};
};


/*
 Правило проходит по всем словам, которые не были найдены в морфологии.
 Смотрит, не начинаются ли они с префикса "пол-" ("полу-"),  если начинаются, и в морфологии есть
 такое же существительное  без приставки, тогда ставим лемму без приставки, а в графем.
 пометы заносим  графету "#ПОЛУ".
 Если слово без приставки стоит в родительном падеже, тогда нужно выставить
 граммем им., вн. и  рд.
 Например,
  Полчаса оказались (им)
  Я прождал полчаса (вн)
  Мечта этого получаса (вн) ??
 Во всех остальных случаях нужно оставить граммемы без изм.
   получасом раньше
*/

/*
  "первые полчаса речи были образцом ораторского мастерства"
  "Прошло полчаса"
   Считаем,	 что все сущесвительные начинающиеся с "пол-" имеют граммему множественного
   числа, чтобы собралась группа "первые полчаса" . Поэтому примеры:
   "Прошло полчаса"
   "Пол-Москвы знает об этом"
   будут не собираться на синтаксисе, но соберутся на семантике,
   где несогласование по роду не является фатальным.
*/

void CMAPost::SemiNouns()
{
	for (CLineIter it = m_Words.begin(); it != m_Words.end(); it++)
	{
		CPostLemWord& W = *it;
		if (!W.HasDes(ORLE)) continue;
		if (!W.m_bPredicted) continue;

		size_t PrefixLen;
		bool bChangeToPlural = false;
		{
			if (W.m_strUpperWord.substr(0, 4) == _R("ПОЛУ"))
				PrefixLen = 4;
			else
				if (W.m_strUpperWord.substr(0, 4) == _R("ПОЛ-"))
				{
					PrefixLen = 4;
					bChangeToPlural = true;
				}
				else
					if (W.m_strUpperWord.substr(0, 3) == _R("ПОЛ"))
					{
						bChangeToPlural = true;
						PrefixLen = 3;
					}
					else
						continue;

		}


		BYTE POS = NOUN;
		{
			std::string WordForm = it->m_strUpperWord;
			WordForm.erase(0, PrefixLen);
			if (WordForm.length() <= 3) continue;
			if (!HasParadigmOfFormAndPoses(WordForm, 1 << POS))
			{
				POS = NUMERAL;
				if (!HasParadigmOfFormAndPoses(WordForm, 1 << POS)) continue; //полмиллиона

			};
		}


		// установка графематической пометы
		W.m_UnparsedGraphemDescriptorsStr += _R(" #ПОЛУ ");

		// установка граммем
		if (bChangeToPlural)
			for (int HomNo = 0; HomNo < W.GetHomonymsCount(); HomNo++)
			{
				CHomonym* pH = W.GetHomonym(HomNo);
				uint64_t Grammems;
				std::string GramCodes;

				bool SingularGenitivFound = false;
				for (int i = 0; i < pH->GetGramCodes().length(); i += 2)
					if (pH->GetGramCodes()[i] != '?')
					{
						m_pRusGramTab->GetGrammems((pH->GetGramCodes().c_str() + i), Grammems);
						if (Grammems & _QM(rSingular))
						{
							// конвертируем во множественное число
							Grammems &= (~_QM(rSingular));
							Grammems |= _QM(rPlural);

							std::string NewGramCode;
							if (m_pRusGramTab->GetGramCodeByGrammemsAndPartofSpeechIfCan(POS, Grammems, NewGramCode))
								GramCodes += NewGramCode;

							if (Grammems & _QM(rGenitiv))
							{
								uint64_t Gram = Grammems & ~_QM(rGenitiv);
								// добавляем винительный  и именительный мн. числа, если нашли родительный
								if (m_pRusGramTab->GetGramCodeByGrammemsAndPartofSpeechIfCan(POS, Gram | _QM(rNominativ), NewGramCode))
									GramCodes += NewGramCode;

								if (m_pRusGramTab->GetGramCodeByGrammemsAndPartofSpeechIfCan(POS, Gram | _QM(rAccusativ), NewGramCode))
									GramCodes += NewGramCode;

							}
						}
						else
							GramCodes += pH->GetGramCodes().substr(i, 2);

					};

				if (!GramCodes.empty())
				{
					pH->SetGramCodes(GramCodes);
					pH->InitAncodePattern();
				}


			}


	}

};




/*
  правило идет  по файлу и ищет словоформу УЖЕ. Если после него нет запятой или слова
  с родительным падежом, тогда из этой словоформы удаляется омоним прилагательного
  сравнительной степени
  Например:
   + сказка о белом уже
   + уже ушел
   - Эта кровать уже, чем диван.
   - Эта кровать уже дивана.
*/
void CMAPost::Rule_UZHE()
{
	for (CLineIter it = m_Words.begin(); it != m_Words.end(); it++)
	{
		CPostLemWord& W = *it;
		if (!W.HasDes(ORLE)) continue;
		if (W.m_strUpperWord != _R("УЖЕ")) continue;

		CLineIter next_it = NextNotSpace(it);

		if ((next_it != m_Words.end())
			&& !next_it->m_bComma
			&& (next_it->GetHomonymByGrammem(rGenitiv) == -1)
			)
		{
			W.KillHomonymOfPartOfSpeech(ADJ_FULL);
		}
	};

};



bool  CMAPost::NounHasObviousPluralContext(CLineIter it)
{
	if (it == m_Words.begin()) return false;
	it--;
	it = BackSpaces(it);
	CPostLemWord& W = *it;
	return          W.GetHomonymByPOS(NUMERAL) != -1
		|| W.GetHomonymByPosesandGrammem((1 << ADJ_FULL) | (1 << NUMERAL_P) | (1 << PRONOUN_P), rPlural) != -1;

};

/*
 Правило идет по неизменяемым сущетствительным, которые заканчиваются на "-о".
 Если слева от него стоит числительное (больше "один"), прил., пор.числ или МС-П
 с неомонимичным множественным числом, тогда выходим из правила.
 Иначе удаляем у него омоним множественного числа.
 -Большие пальто
 -их пальто
 +большое пальто
 +первое пальто
 +мое пальто
*/
/*
	это же правило работает для существительных, которые предсказаны
	по шаблону НЕУБИВАЙМЕНЯ (код "яя"), если они заканчиваются на "-o".
	Для того чтобы оотконвертировть эти существительные в единственное число,
	нужно было специально ввести код "яю" ("яю"+ "мн" = "яя")
*/

void CMAPost::Rule_Ideclinable()
{
	for (CLineIter it = m_Words.begin(); it != m_Words.end(); it++)
	{
		CPostLemWord& W = *it;
		if (!W.HasDes(ORLE)) continue;
		if (!W.HasPos(NOUN)) continue;
		if (!endswith(W.m_strUpperWord, _R("О"))) continue;
		for (int i = 0; i < W.GetHomonymsCount(); i++)
		{
			CHomonym* pH = W.GetHomonym(i);
			if ((pH->m_iGrammems & rAllCases) != rAllCases) continue;
			if ((pH->m_iGrammems & rAllNumbers) != rAllNumbers) continue;

			std::string GramCodes = pH->GetGramCodes();
			// анкод должен быть только один, поскольк это неизменяемое существительное
			if (GramCodes.length() != 2) continue;
			if (!NounHasObviousPluralContext(it))
			{
				// отрубаем множественное число
				std::string NewGramCode;
				if (m_pRusGramTab->GetGramCodeByGrammemsAndPartofSpeechIfCan(NOUN, pH->m_iGrammems & ~_QM(rPlural), NewGramCode))
				{
					pH->SetGramCodes(NewGramCode);
					pH->InitAncodePattern();
				}
				else
				{
					// например у фамилий на -о нет варианта с ед. числом
				}
			};
		}

	};
};


/*
	Правило идет по всем леммам. Если лемма была найдена в перечне слов,  у которых затруднена
	множественная форма и слово не находится в "объективном множественном контексте", тогда
	удаляем все грамкоды множественного числа у этой леммы.
*/
void CMAPost::Rule_DeadPlurals()
{
	for (CLineIter it = m_Words.begin(); it != m_Words.end(); it++)
	{
		CPostLemWord& W = *it;
		if (!W.HasPos(NOUN)) continue;
		if (NounHasObviousPluralContext(it)) continue;
		if (W.m_bPredicted) continue;
		if (!W.HasGrammem(rSingular)) continue; // only pluralia 
		for (int i = 0; i < W.GetHomonymsCount(); i++)
		{
			CHomonym* pH = W.GetHomonym(i);
			pH->m_bDelete = false;
			if (pH->HasGrammem(rDeFactoSingTantum))
				if (!pH->DeleteAncodesByGrammemIfCan(rPlural))
					pH->m_bDelete = true;
		};
		W.SafeDeleteMarkedHomonyms();

	};

};

/*
	Правило идет по всем словам. Если слово омонимично, тогда уничтожаются все омонимы,
	кратких и сравнительных форм относительных прилагательных. Например, для формы "стоял"
	будет уничтожена краткая форма от прилагательного "стоялый", а останется только вариант
	"стоять".

*/
void CMAPost::Rule_RelationalAdjective()
{
	for (CLineIter it = m_Words.begin(); it != m_Words.end(); it++)
	{
		CPostLemWord& W = *it;
		if (((W.GetPoses() & ((1 << ADJ_SHORT) | (1 << ADJ_FULL)))) == 0) continue;

		for (int i = 0; i < W.GetHomonymsCount(); i++)
		{
			CHomonym* pH = W.GetHomonym(i);
			pH->m_bDelete = false;
			if (pH->m_iPoses & ((1 << ADJ_SHORT) | (1 << ADJ_FULL)))
			{
				if (!pH->HasGrammem(rQualitative))
				{

					// только относительные прилагательные
					if (pH->HasGrammem(rComparative))
						if (!pH->DeleteAncodesByGrammemIfCan(rComparative))
							pH->m_bDelete = true; // если не получилось отредактировать анкоды, попробуем адалить весь омоним
					if (pH->HasPos(ADJ_SHORT))
						pH->m_bDelete = true;
				}
			}
		}
		W.SafeDeleteMarkedHomonyms();
	};

};




enum QuoteMarkEnum { QuoteMark, EOS };
struct CQuoteMark
{
	QuoteMarkEnum m_TokenType;
	CLineIter     m_LineIter;
	bool m_bFirstBeforeLastQuote;
	CQuoteMark()
	{
		m_bFirstBeforeLastQuote = false;
	};
};


/*
в каждом предложении отедльно удаляет кавычки, попутно считая их, всем словам между четной кавычкой (открывающей) и нечетной кавычкой
(закрывающей) ставит помету #QUOTED.  Если для открывающей кавычки Х не было найдено закрывающей до конца предложения,
тогда считается, что последняя
кавычка в предложении закрывает две предыдущих кавычки, например:
Мама сказала: "Я презираю "Единую Россию".

*/
void CMAPost::Rule_QuoteMarks()
{
	//ставим #QUOTED
	size_t QuoteNo = 0;
	CLineIter prev_prev_quote_it = m_Words.end();
	CLineIter prev_quote_it = m_Words.end();

	for (CLineIter it = m_Words.begin(); it != m_Words.end(); it++)
	{
		CPostLemWord& W = *it;
		if (W.m_bQuoteMark)
		{
			if ((QuoteNo % 2) == 1)
			{
				assert(prev_quote_it != m_Words.end());
				for (CLineIter itt = prev_quote_it; itt != it; itt++)
					itt->AddDes(OQuoted);
			}
			QuoteNo++;

			prev_prev_quote_it = prev_quote_it;
			prev_quote_it = it;
		}
		if (W.HasDes(OSentEnd))
		{
			if (((QuoteNo % 2) == 1) && QuoteNo > 1)
			{
				assert(prev_prev_quote_it != m_Words.end());
				assert(prev_quote_it != m_Words.end());
				// есть открывающая, но нет закрывающей
				for (CLineIter itt = prev_prev_quote_it; itt != prev_quote_it; itt++)
					itt->AddDes(OQuoted);
			}
			QuoteNo = 0;
		}
	}

	// Удаление кавычек
	CLineIter prev_it = m_Words.begin();
	for (CLineIter it = m_Words.begin(); it != m_Words.end(); it++)
	{
		for (; (it != m_Words.end()) && it->m_bQuoteMark; it = m_Words.erase(it))
			if (it->HasDes(OSentEnd))
				prev_it->AddDes(OSentEnd);

		if (it == m_Words.end()) break;
		if (!it->m_bSpace)
			prev_it = it;
	}


};


/*
приписывает всем ИЛЕ морфологическую интерпретаций НЕУБИВАЙМЕНЯ
*/
void CMAPost::Rule_ILE()
{
	for (CLineIter it = m_Words.begin(); it != m_Words.end(); it++)
	{
		CPostLemWord& W = *it;
		if (W.HasDes(OLLE))
		{
			W.DeleteAllHomonyms();
			CHomonym* pNew = W.AddNewHomonym();
			pNew->SetMorphUnknown();
			pNew->m_strLemma = W.m_strUpperWord;
			pNew->m_CommonGramCode = _R("Фа");
			//if ( W.HasDes(OUp) ) 
			//	pNew->m_CommonGramCode += _R("аоатац");//m_DURNOVOGramCode;
			pNew->SetGramCodes(m_pRusGramTab->GetGramCodes(NOUN, rAllCases | rAllNumbers | rAllGenders, 0));//m_DURNOVOGramCode;//"Йшааабавагадаеажазаиайакаласгагбгвгггдгегжгзгигйгкглеаебевегедееежезеиейекелижизииийикил";
			pNew->InitAncodePattern();
		}
	};
};


/*
  ищет последовательность КАК МОЖНО х, где  у х есть омоним [П сравн],
  удаляет леммы КАК  и МОЖНО,   оставляет у х омоним  только на [П сравн].
  Добавляет  х  графету  #КАК_МОЖНО.
*/
void CMAPost::Rule_KAK_MOZHNO()
{
	for (CLineIter it = m_Words.begin(); it != m_Words.end(); it++)
	{
		// ищем словоформу _R("как")
		CPostLemWord& W = *it;
		if (!W.HasDes(ORLE)) continue;
		if (W.m_strUpperWord != _R("КАК")) continue;
		if (W.HasOborot1() != W.HasOborot2()) continue;// "т.е." - это оборот, его обрабатывать не надо

		// ищем словоформу _R("можно")
		CLineIter mozno_it = NextNotSpace(it);
		if (mozno_it == m_Words.end()) break;
		if (mozno_it->m_strUpperWord != _R("МОЖНО")) continue;

		// ищем [П сравн]
		CLineIter compar_it = NextNotSpace(mozno_it);
		if (compar_it == m_Words.end()) break;
		int iHom = compar_it->GetHomonymByGrammem(rComparative);
		if (compar_it->m_strUpperWord == _R("БОЛЕЕ") || compar_it->m_strUpperWord == _R("МЕНЕЕ"))
			iHom = compar_it->GetHomonymByPOS(ADV);
		if (iHom == -1) continue;
		if (compar_it->HasOborot1() != compar_it->HasOborot2()) continue;// "т.е." - это оборот, его обрабатывать не надо

		//оставляем только омоним [П сравн]
		compar_it->SetAllOtherHomsDel(iHom);
		compar_it->DeleteMarkedHomonymsBeforeClauses();
		compar_it->m_UnparsedGraphemDescriptorsStr += _R(" #КАК_МОЖНО ");
		m_Words.erase(it);
		m_Words.erase(mozno_it);
		it = compar_it;
	}


};

bool CMAPost::CanBeDubleDelimiter(CLineIter it)
{
	const std::string& WordForm = it->m_strUpperWord;

	return ((WordForm == _R("ДА"))
		|| (WordForm == ",")
		|| (WordForm == _R("И"))
		|| (WordForm == _R("-"))
		);
};

void CMAPost::Rule_Redublication()
{
	for (CLineIter it = m_Words.begin(); it != m_Words.end(); it++)
	{
		CPostLemWord& W = *it;
		if (!W.HasDes(ORLE)) continue;
		// если попали в оборот, надо выйти,например, "идем вниз, вниз по лестнице"
		if (W.IsInOborot()) continue;
		CLineIter last_it = it;
		bool bHasEndOfSent = false;
		while (true)
		{
			CLineIter delimiter_iter = NextNotSpace(last_it);
			if (delimiter_iter == m_Words.end()) break;
			if (!CanBeDubleDelimiter(delimiter_iter)) break;
			bHasEndOfSent |= delimiter_iter->HasDes(OSentEnd);

			CLineIter duble_word = NextNotSpace(delimiter_iter);
			if (duble_word == m_Words.end()) break;
			bHasEndOfSent |= duble_word->HasDes(OSentEnd);
			if (duble_word->m_strUpperWord == _R("НЕ"))
			{
				duble_word = NextNotSpace(duble_word);
				if (duble_word == m_Words.end()) break;
				bHasEndOfSent |= duble_word->HasDes(OSentEnd);
			};
			// если попали в оборот, надо выйти,например, "идем вниз, вниз по лестнице"
			if (duble_word->IsInOborot())
				break;
			// это основное  сравнение редупликации!
			if (W.m_strUpperWord != duble_word->m_strUpperWord) break;

			last_it = duble_word;
		}

		if (last_it != it)
		{
			CLineIter itt = it;
			itt++;
			while (itt != last_it)
				itt = m_Words.erase(itt);
			m_Words.erase(last_it);
			W.m_UnparsedGraphemDescriptorsStr += _R(" #РЕДУПЛ ");
			if (bHasEndOfSent)
				W.AddDes(OSentEnd);
		}
	}

};

CLineIter CMAPost::PassSpaces(CLineIter it)
{
	while (it != m_Words.end())
	{
		if (!it->m_bSpace) break;
		it++;
	};
	return it;

};

CLineIter CMAPost::NextNotSpace(CLineIter it)
{
	if (it == m_Words.end()) return it;
	it++;
	return PassSpaces(it);
};

CLineIter CMAPost::BackSpaces(CLineIter it)
{
	if (it == m_Words.end()) it--;

	while (it != m_Words.begin())
	{
		if (!it->m_bSpace) break;
		it--;
	};
	return it;

};

void CMAPost::Rule_CHTO_ZA()
{
	for (CLineIter it = m_Words.begin(); it != m_Words.end(); it++)
	{
		CPostLemWord& W = *it;
		if (!W.HasDes(ORLE)) continue;
		if (W.m_strUpperWord != _R("ЧТО")) continue;
		if (W.HasOborot1() != W.HasOborot2())  continue;

		CLineIter za_it = NextNotSpace(it);
		if (za_it == m_Words.end()) return;
		if (za_it->m_strUpperWord != _R("ЗА")) continue;
		if (za_it->HasOborot1() != za_it->HasOborot2())  continue;

		CLineIter noun_it = za_it;
		do
		{
			noun_it = NextNotSpace(noun_it);
			if (noun_it == m_Words.end()) return;
			if (!m_pRusGramTab->is_left_noun_modifier(noun_it->GetPoses(), 0)) break;
		} while (!noun_it->HasPos(NOUN));

		if (!noun_it->HasPos(NOUN)) break;


		std::vector<CFormInfo> Kakoi;
		std::string a = _R("какой");
		m_pRusLemmatizer->CreateParadigmCollection(true, a, false, false, Kakoi);
		if (Kakoi.empty()) return;

		std::string GramCodes;
		std::string WordForm;
		for (int j = 0; j < Kakoi[0].GetCount(); j++)
			for (int k = 0; k < noun_it->GetHomonymsCount(); k++)
			{
				const CHomonym* pNoun = noun_it->GetHomonym(k);
				if (m_pRusGramTab->GleicheGenderNumberCase(pNoun->m_CommonGramCode.c_str(), pNoun->GetGramCodes().c_str(), Kakoi[0].GetAncode(j).c_str()))
				{
					GramCodes += Kakoi[0].GetAncode(j);
					WordForm = Kakoi[0].GetWordForm(j);
				}
			}
		W.DeleteAllHomonyms();
		CHomonym* pNew = W.AddNewHomonym();
		pNew->SetHomonym(&Kakoi[0]);
		pNew->SetGramCodes(GramCodes);
		RmlMakeLower(WordForm, morphRussian);
		W.SetWordStr(WordForm, morphRussian);
		W.DeleteOborotMarks();
		m_Words.erase(za_it);
	};
};


void CMAPost::InsertComma(CLineIter it)
{
	CPostLemWord P(m_pRusGramTab);
	int dummy;
	P.ProcessPlmLineForTheFirstHomonym(", 0 0 PUN", morphRussian, dummy);
	m_Words.insert(it, P);
};

/*
 Ищем  конструкцию типа "вот уже ... как ...",
 например, "вот уже несколько лет как он уехал".
 Переводим  ее в конструкцию
	"вот уже несколько лет прошло, как он уехал".
*/

void CMAPost::Rule_VOT_UZHE()
{

	for (CLineIter it = m_Words.begin(); it != m_Words.end(); it++)
	{
		CPostLemWord& W = *it;
		if (!W.HasDes(ORLE)) continue;
		if (W.m_strUpperWord != _R("ВОТ")) continue;

		CLineIter next_it = NextNotSpace(it);
		if (next_it == m_Words.end()) return;
		if (next_it->m_strUpperWord != _R("УЖЕ")) continue;


		//	дальше десяти слов не надо 
		bool bFound = false;
		int WordCount = 0;
		for (; (WordCount < 10) && (next_it != m_Words.end()); next_it++)
		{
			if (next_it->m_strUpperWord == _R("КАК"))
			{
				bFound = true;
				break;
			};
			if (m_pRusGramTab->IsStrongClauseRoot(next_it->GetPoses())) break;
			WordCount++;
		};
		if (!bFound) break;

		CPostLemWord P(m_pRusGramTab);
		P.m_bWord = true;
		P.m_TokenLengthInFile = 0;
		P.AddDes(ORLE);
		P.AddDes(OLw);
		P.LemmatizeForm(_R("прошли"), m_pRusLemmatizer);
		P.SetWordStr(_R("прошли"), morphRussian);
		m_Words.insert(next_it, P);
		InsertComma(next_it);
	};
}



/*
  программа ищет неузнанные слова, если они написаны с большой буквы, тогда
  оставляем у этого существительно только интерпретацию существительного
	Например:
	 в районе населенного пункта Устьпорочи
	 под горой Рай-Из
*/

void CMAPost::Rule_UnknownNames()
{
	for (CLineIter it = m_Words.begin(); it != m_Words.end(); it++)
	{
		CPostLemWord& W = *it;
		if (!W.HasDes(ORLE)) continue;
		if (!W.m_bPredicted)  continue;
		if (!W.m_bFirstUpperAlpha) continue;
		for (int i = 0; i < W.GetHomonymsCount(); i++)
			W.GetHomonym(i)->m_bDelete = !W.GetHomonym(i)->HasPos(NOUN);
		W.SafeDeleteMarkedHomonyms();
	};
};


/*
	программа ищет словоформу _R("сорок"), если после нее идет
	числительное, тогда  программа уничтожает омоним _R("сорока") у этой словоформы
	+ триста сорок пятый
	+ сорок один
	? триста сорок пятого убили (триста сорок(птиц) убили пятого)

	так же для _R("семью"): "иметь семью" - здесь не наречие
*/


void CMAPost::Rule_SOROK()
{
	for (CLineIter it = m_Words.begin(); it != m_Words.end(); it++)
	{
		CPostLemWord& W = *it;
		if (!W.HasDes(ORLE)) continue;
		if (!W.HasPos(NUMERAL)) continue;
		W.SetHomonymsDel(false);
		for (int i = 0; i < W.GetHomonymsCount(); i++)
		{
			CHomonym* pH = W.GetHomonym(i);
			if (pH->m_strLemma == _R("СОРОКА"))
			{
				CLineIter next_it = NextNotSpace(it);
				if (next_it == m_Words.end()) return;
				if (next_it->GetPoses() & ((1 << NUMERAL) | (1 << NUMERAL_P)))
					pH->m_bDelete = true;
			}
			if (pH->m_strLemma == _R("СЕМЬЮ"))
			{
				CLineIter next_it = NextNotSpace(it);
				if (next_it == m_Words.end() || !(next_it->GetPoses() & ((1 << NUMERAL) | (1 << NUMERAL_P))))
					pH->m_bDelete = true;
			}
		}
		W.SafeDeleteMarkedHomonyms();
	}
};


/*
* правило  заменяет анкод неизменяемого существительного на все анкоды всех падежей, чисел и родов
*/
void CMAPost::Rule_ExpandIndeclinableGramcodes()
{
	for (CLineIter it = m_Words.begin(); it != m_Words.end(); it++)
	{
		CPostLemWord& W = *it;
		if (!W.HasDes(ORLE)) continue;

		//ЦБ      С ср,0 -> C ср,им  ср,вн ср,дт
		for (int HomNo = 0; HomNo < W.GetHomonymsCount(); HomNo++) {
			if (W.GetHomonym(HomNo)->HasPos(NOUN) && W.GetHomonym(HomNo)->HasGrammem(rIndeclinable) && !W.GetHomonym(HomNo)->HasGrammem(rInitialism))
			{
				CHomonym* pH = W.GetHomonym(HomNo);
				pH->m_CommonGramCode += pH->GetGramCodes();
				pH->SetGramCodes(m_pRusGramTab->GleicheAncode1(GenderNumber0,
					m_pRusGramTab->GetGramCodes(NOUN, rAllCases | rAllNumbers | rAllGenders, 0), pH->GetGramCodes()));  //rAllNumbers, AnCodes, _QM(rSingular));
			}
		}
	}
}


/*
	программа ищет словоформы, которые не были написаны только заглавными буквами,
	если этому слову приписан омоним с граммемой  "аббр", тогда  этот омоним удаляется.
	Например, для слова "под" будет удален омоним "ПОД" ("поисковый образ документа"), который  должен был быть написан
	большими буквами.
*/


void CMAPost::Rule_Abbreviation()
{
	for (CLineIter it = m_Words.begin(); it != m_Words.end(); it++)
	{
		CPostLemWord& W = *it;
		if (!W.HasDes(ORLE)) continue;

		if (W.HasDes(OUp)) continue;

		for (int HomNo = 0; HomNo < W.GetHomonymsCount(); HomNo++)
			if (W.GetHomonym(HomNo)->HasGrammem(rInitialism))
				if (W.GetHomonymsCount() > 1 && W.GetHomonym(HomNo)->m_strLemma == W.m_strUpperWord) //свыше 50 проц
					W.EraseHomonym(HomNo);
				else
				{
					CHomonym* pH = W.GetHomonym(HomNo);
					std::vector<CFormInfo> Paradigms;
					CFormInfo P;
					std::string AnCodes;
					m_pRusLemmatizer->CreateParadigmFromID(pH->m_lPradigmID, P);
					pH->m_CommonGramCode += pH->GetGramCodes();
					AnCodes = pH->GetGramCodes();
					pH->SetGramCodes("");
					std::string xx = P.GetAncode(0);
					for (long k = 0; k < P.GetCount(); k++)
						if (m_pRusGramTab->GleicheAncode1(0, pH->m_CommonGramCode + pH->GetGramCodes(), P.GetAncode(k)) == ""
							&& (m_pRusGramTab->GetAllGrammems(P.GetAncode(k).c_str()) & m_pRusGramTab->GetAllGrammems(AnCodes.c_str())) == m_pRusGramTab->GetAllGrammems(P.GetAncode(k).c_str())
							&& m_pRusGramTab->GetPartOfSpeech(P.GetAncode(k).c_str()) == m_pRusGramTab->GetPartOfSpeech(AnCodes.c_str())
							)
							pH->SetGramCodes(pH->GetGramCodes() + P.GetAncode(k));
				}
	};
};


/*
	программа ищет  последовательности типа "по-восточному", "по-ковбойски", ("по" + форма прилагательного)
	объединяет их в одно слово, если еще этого не сделала морфология,
	говорит, что это наречие.
*/


void CMAPost::Rule_AdverbFromAdjectives()
{
	for (CLineIter it = m_Words.begin(); it != m_Words.end(); it++)
	{
		CPostLemWord& W = *it;
		if (!W.HasDes(ORLE)) continue;
		if (!W.m_bPredicted)    continue;
		if (W.m_strUpperWord.length() < 3) continue;
		if (W.m_strUpperWord.substr(0, 3) != _R("ПО-")) continue;
		if (!HasParadigmOfFormAndPoses(W.m_strUpperWord.substr(3), (1 << ADJ_FULL) | (1 << ADJ_SHORT)))
			continue;
		W.DeleteAllHomonyms();
		CHomonym* pNew = W.AddNewHomonym();
		pNew->m_strLemma = it->m_strUpperWord;
		pNew->SetMorphUnknown();
		std::string NewGramCode;
		m_pRusGramTab->GetGramCodeByGrammemsAndPartofSpeechIfCan(ADV, 0, NewGramCode);
		pNew->SetGramCodes(NewGramCode);
		pNew->InitAncodePattern();
	}

};



void CMAPost::SaveToFile(std::string s)
{
	/*FILE * fp = fopen (s.c_str(), "w");
	assert (fp);
	if (!fp) return;
	for (CLineIter it=m_Words.begin(); it !=  m_Words.end(); it++)
	{
		fprintf (fp, "%s\n", it->GetPlmStr()GetStr().c_str());
	};
	fclose(fp);*/

};


/*
	если слово  пришло из словаря имен, и было написано с маленькой  буквы
	нужно удалить такую интерпретацию
*/
void CMAPost::Rule_FilterProperName()
{
	for (CLineIter it = m_Words.begin(); it != m_Words.end(); it++)
	{
		CPostLemWord& W = *it;
		if (!W.HasDes(OLw)) continue;
		for (int i = 0; i < W.GetHomonymsCount(); i++)
			W.GetHomonym(i)->m_bDelete = ((W.GetHomonym(i)->m_iGrammems | W.GetHomonym(i)->m_TypeGrammems) & (_QM(rSurName) | _QM(rName) | _QM(rPatronymic) | _QM(rToponym))) > 0;
		W.SafeDeleteMarkedHomonyms();
	}
}

/*
	если слово  является отчеством, тогда его лемма будет именем,
	например Иванович -> Иван
	Процедура заменяет лемму "Иван" на нормальную форму  отчества.
	Например, Ивановичем -> Иванович
			  Ивановной  -> Ивановна
*/
void CMAPost::Rule_ChangePatronymicLemmas()
{

	for (CLineIter it = m_Words.begin(); it != m_Words.end(); it++)
	{
		CPostLemWord& W = *it;
		if (!W.HasDes(ORLE)) continue;
		int HomNo = W.GetHomonymByGrammem(rPatronymic);
		if (HomNo == -1) continue;
		CHomonym* pH = W.GetHomonym(HomNo);

		std::vector<CFormInfo> Paradigms;
		std::string Word = W.m_strWord;
		m_pRusLemmatizer->CreateParadigmCollection(false, Word, true, true, Paradigms);
		for (long k = 0; k < Paradigms.size(); k++)
			if (Paradigms[k].GetSrcAncode() == pH->GetGramCodes())
			{
				for (size_t j = 0; j < Paradigms[k].GetCount(); j++)
				{
					uint64_t g;
					m_pRusGramTab->GetGrammems(Paradigms[k].GetAncode((uint16_t)j).c_str(), g);
					if ((g & _QM(rPatronymic))
						&& (g & _QM(rNominativ))
						&& (g & _QM(rSingular))
						&& (rAllGenders & g & pH->m_iGrammems)
						)
					{
						pH->m_strLemma = Paradigms[k].GetWordForm(j);
						break;
					}
				};
				break;

			};


	}

};


void CMAPost::SolveAmbiguityUsingRuleForTwoPredicates(CLineIter start, CLineIter end)
{
	for (CLineIter root = start; root != end; root++)
	{
		CPostLemWord& RootWord = *root;
		if (RootWord.GetHomonymsCount() != 1 || !m_pRusGramTab->IsStrongClauseRoot(RootWord.GetHomonym(0)->m_iPoses)) continue;

		bool bHasAnalytical = false;
		for (CLineIter it = start; it != end; it++)
		{
			CPostLemWord& W = *it;
			// аналитические  формы еще не собраны, мы не имеем  право убивать омонимы у вспом. глаголов
			bHasAnalytical = bHasAnalytical || W.HasAnalyticalBe();
		}

		for (CLineIter it = start; it != end; it++)
		{
			CPostLemWord& W = *it;
			if (W.HasAnalyticalBe() || it == root) continue;

			if (W.GetHomonymsCount() > 1 && m_pRusGramTab->IsStrongClauseRoot(W.GetPoses()))
			{
				W.SetHomonymsDel(false);
				for (int i = 0; i < W.GetHomonymsCount(); i++)
				{
					CHomonym* pH = W.GetHomonym(i);
					if (bHasAnalytical)
						if (pH->HasPos(PARTICIPLE_SHORT)
							|| pH->HasPos(ADJ_SHORT)
							|| pH->HasPos(PREDK)
							)
							continue;

					pH->m_bDelete = m_pRusGramTab->IsStrongClauseRoot(pH->m_iPoses);
				}
				W.SafeDeleteMarkedHomonyms();
			}
		}
		break;
	}
}

void CMAPost::Rule_TwoPredicates()
{
	CLineIter start = m_Words.begin();
	size_t  PeriodSize = 0;
	for (CLineIter it = m_Words.begin(); it != m_Words.end(); it++)
	{
		CPostLemWord& W = *it;
		if (W.HasDes(OSentEnd) || W.HasDes(OPun) || W.m_strUpperWord == _R("И") || W.m_strUpperWord == _R("ИЛИ"))
		{
			CLineIter end_it = it;
			if (W.HasDes(OSentEnd))
				end_it++;
			SolveAmbiguityUsingRuleForTwoPredicates(start, end_it);
			start = it;
			PeriodSize = 0;
		}
		if (PeriodSize > 10)
			start++;
		else
			PeriodSize++;
	}
	if (PeriodSize > 1)
		SolveAmbiguityUsingRuleForTwoPredicates(start, m_Words.end());
}



bool CMAPost::FilterOnePostLemWord(CPostLemWord& W, uint16_t tagid1, uint16_t tagid2) const
{
	std::vector<CTag> Tags = m_TrigramModel.m_TagSet.DecipherTagStr(m_TrigramModel.m_RegisteredTags[tagid1], m_pRusGramTab);
	if (tagid2 != UnknownTag)
	{
		std::vector<CTag> Tags2 = m_TrigramModel.m_TagSet.DecipherTagStr(m_TrigramModel.m_RegisteredTags[tagid2], m_pRusGramTab);
		Tags.insert(Tags.end(), Tags2.begin(), Tags2.end());
	}

	std::set<std::string> Lemmas;
	for (size_t i = 0; i < W.GetHomonymsCount(); i++)
	{
		CHomonym* pH = W.GetHomonym(i);
		bool goodHomonym = m_TrigramModel.FindGramTabLineInTags(Tags, pH->m_iPoses, pH->m_iGrammems | pH->m_TypeGrammems);
		if ((pH->m_iPoses & (_QM(PRONOUN_PREDK) | _QM(PREDK) | _QM(PARTICLE) | _QM(PREP) | _QM(CONJ))) > 0) {
			goodHomonym = true; // Trigram is bad for auxiliary parts of speech
		}
		if (pH->m_iGrammems & _QM(rImpersonal)) {
			goodHomonym = true; // Ни у кого не хватило духу сказать ему об этом
		}
		if (goodHomonym) {
			Lemmas.insert(pH->m_strLemma);
		}
		else {
			pH->m_bDelete = true;
		}
	}
	for (size_t i = 0; i < W.GetHomonymsCount(); i++)
	{
		CHomonym* pH = W.GetHomonym(i);
		if (Lemmas.find(pH->m_strLemma) == Lemmas.end()) {
			pH->m_bDelete = true;
		}
	}
	W.SafeDeleteMarkedHomonyms();
	return true;
}

bool CMAPost::FilterPostMorphWords()
{
	std::vector<std::string> tokens;
	std::vector<CWordIntepretation> tags;
	std::list<CPostLemWord>::iterator sent_start = m_Words.begin();
	for (std::list<CPostLemWord>::iterator it = m_Words.begin(); it != m_Words.end(); )
	{
		if (it->HasDes(OSentEnd) || (tokens.size() > 150))
		{
			tokens.push_back(it->m_strWord);
			if (!m_TrigramModel.viterbi(tokens, tags))
				return false;
			size_t WordNo = 0;
			it++;
			for (std::list<CPostLemWord>::iterator it2 = sent_start; it2 != it; it2++)
			{
				assert(WordNo < tags.size());
				CPostLemWord& w = *it2;
				if (w.GetHomonymsCount() > 1) {
					FilterOnePostLemWord(w, tags[WordNo].m_TagId1, tags[WordNo].m_TagId2);
				}
				WordNo++;
			}



			tokens.clear();
			tags.clear();
			sent_start = it;
		}
		else
		{
			tokens.push_back(it->m_strWord);
			it++;
		}

	}
	return true;
}

project (SrcBinDict)

macro (Convertor BinaryConvertor Source BinData)
    list(APPEND AllBinFiles ${BinData})
    add_custom_command(
        OUTPUT ${BinData}
        COMMAND ${BinaryConvertor} ${Source} ${BinData}
        #DEPENDS ${BinaryConvertor} ${Source} 
        DEPENDS ${Source} MorphDicts
        COMMENT "${BinaryConvertor} ${Source} ${BinData}"
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        VERBATIM
    )
endmacro(Convertor)

Convertor(AprDictGen dict2809.txt ${PROJECT_SOURCE_DIR}/../BinDict/dict.bin)
Convertor(asp_read ASP.txt ${PROJECT_SOURCE_DIR}/../BinDict/asp_dict.bin)
Convertor(deriv_read troiki_suff ${PROJECT_SOURCE_DIR}/../BinDict/deriv_suff.bin)
Convertor(deriv_read troiki_pref ${PROJECT_SOURCE_DIR}/../BinDict/deriv_pref.bin)

#not used
#Convertor(GenSynDict synonyms.txt ${PROJECT_SOURCE_DIR}/../BinDict/synonyms.bin)


macro (GenFreqDictConv Source BinData)
    list(APPEND AllBinFiles ${BinData})
    add_custom_command(
        OUTPUT ${BinData}
        COMMAND GenFreqDict ${Source} ${BinData} shira.txt
        DEPENDS GenFreqDict ${Source} shira.txt ../BinDict/dict.bin
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        COMMENT "GenFreqDict ${Source} ${BinData} shira.txt" 
        VERBATIM
    )
endmacro(GenFreqDictConv)

GenFreqDictConv (../../Texts/freq_comp.txt ${PROJECT_SOURCE_DIR}/../BinDict/freq_comp.bin)
GenFreqDictConv (../../Texts/freq_fin.txt ${PROJECT_SOURCE_DIR}/../BinDict/freq_fin.bin)
GenFreqDictConv (../../Texts/freq_hud.txt ${PROJECT_SOURCE_DIR}/../BinDict/freq_hud.bin)


add_custom_target( 
    BinDicts
    SOURCES ${AllBinFiles}
)




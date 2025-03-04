//================================
// ==========   Dialing Lemmatizer (www.aot.ru)
//================================

#include "../common/utilit.h"
#include "../common/argparse.h"
#include "../SynanLib/SyntaxHolder.h"


#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <locale.h>



void initArgParser(int argc, const char **argv, ArgumentParser& parser) {
    parser.AddOption("--help");
    parser.AddArgument("--language", "language");
    parser.AddArgument("--input-file", "specify file for input text in utf-8", true);
    parser.AddArgument("--output-file", "specify file for tokens table", true);
    parser.AddArgument("--input-file-mask", "c:/*.txt", true);
    parser.AddOption("--print-ancodes");
    parser.Parse(argc, argv);
}

int main(int argc, const char **argv) {
    ArgumentParser args;
    initArgParser(argc, argv, args);
    MorphLanguageEnum  language = args.GetLanguage();
    CSyntaxHolder H;
    if (!H.LoadSyntax(args.GetLanguage())) {
        std::cerr << "initialization error\n";
        return 1;
    };
    H.m_pPostMorph->m_bHumanFriendlyOutput = !args.Exists("print-ancodes");
    std::vector <std::pair<std::string, std::string> > file_pairs;

    if (args.Exists("input-file-mask")) {
        auto file_names = list_path_by_file_mask(args.Retrieve("input-file-mask"));
        for (auto filename : file_names) {
            file_pairs.push_back({ filename, filename + ".mapost" });
        }
    }
    else {
        file_pairs.push_back({ args.Retrieve("input-file"), args.Retrieve("output-file") });
    }

    for (auto& p : file_pairs) {
        int dummy;
        if (!H.GetMorphology(p.first, true, dummy)) {
            return 1;
        }

#ifdef _DEBUG
        H.m_PlmLines.SaveToFile("before.lem");
#endif

        CPlmLineCollection MapostPlmLines;
        if (!H.RunMapost(MapostPlmLines)) {
            return 1;
        }
        MapostPlmLines.SaveToFile(p.second);
    }
    return 0;
}

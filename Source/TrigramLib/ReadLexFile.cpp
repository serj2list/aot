#include "TrigramModel.h"
#include "../common/bserialize.h"

inline size_t get_size_in_bytes (const CLexProb& t)
{
	return sizeof(t.m_PrevTag)
        + sizeof(t.m_Tag)
        + sizeof(t.m_Count);
};
inline size_t save_to_bytes(const CLexProb& t, BYTE* buf)
{
	buf += save_to_bytes(t.m_PrevTag, buf);
	buf += save_to_bytes(t.m_Tag, buf);
	buf += save_to_bytes(t.m_Count, buf);
	return get_size_in_bytes(t);
}

inline size_t restore_from_bytes(CLexProb& t, const BYTE* buf)
{
	buf += restore_from_bytes(t.m_PrevTag, buf);
	buf += restore_from_bytes(t.m_Tag, buf);
	buf += restore_from_bytes(t.m_Count, buf);
	return get_size_in_bytes(t);
};




bool CTrigramModel::read_dictionary_text_file(std::map<std::string,  std::vector<CLexProb> >& Dictionary) const
{
	FILE* fp = fopen(m_DictionaryFile.c_str(), "r");
	if (!fp) 
	{
		fprintf (stderr, "cannot open %s\n", m_DictionaryFile.c_str());
		return false;
	};
	size_t line_no = 0;
	assert  (m_TagsCount != UnknownTag );
	const size_t buffer_size = 50000;
	char buffer[buffer_size];
	while (fgets(buffer, buffer_size, fp))
	{
		line_no ++;
		StringTokenizer tok(buffer, "\t \r\n");
		if (!tok())
		{
			 fprintf(stderr, "empty line found in %s\n", m_DictionaryFile.c_str()); 
			 return false;
		};
        std::string WordStr = tok.val();
		std::vector<CLexProb> wd;
		if (!m_TestLexicon.empty())
		{
			if (m_TestLexicon.find(WordStr) == m_TestLexicon.end())
				continue;
		}
		if (Dictionary.find(WordStr) != Dictionary.end())
		{
			fprintf(stderr, "duplicate dictionary entry \"%s\" (%s:%li)\n", WordStr.c_str(), m_DictionaryFile.c_str(), line_no);
			return false;
		};
		
		while ( tok () )
		{
			std::string tag1 = tok.val();
			uint16_t PrevTag = find_tag(tag1 );
			if (PrevTag == UnknownTag)
			{	
				fprintf(stderr, "invalid tag \"%s\" (%s:%li)\n", tag1.c_str(), m_DictionaryFile.c_str(), line_no); 
				return false;
			}
			if (!tok ())
			{
				fprintf(stderr, "can't find the second tag (%s:%li)\n", m_DictionaryFile.c_str(), line_no);
				return false;
			};
			std::string tag2 = tok.val();
			uint16_t CurrTag = find_tag(tag2 );
			if (CurrTag == UnknownTag)
			{	
				fprintf(stderr, "invalid tag \"%s\" (%s:%li)\n", tag2.c_str(), m_DictionaryFile.c_str(), line_no); 
				return false;
			}
			if (!tok ())
			{
				fprintf(stderr, "can't find tag count (%s:%li)\n", m_DictionaryFile.c_str(), line_no);
				return false;
			};

			int cnt = atoi(tok.val());
			assert (cnt != 0);
			wd.push_back(CLexProb(PrevTag,CurrTag, cnt));
		}
		Dictionary[WordStr] = wd;
	}

	fprintf(stderr,"read %d entries (type/token) from dictionary\n",Dictionary.size());
	fclose (fp);

	return true;
}

bool CTrigramModel::write_dictionary_binary(const std::map<std::string, std::vector<CLexProb> >& Dictionary) const
{
    std::string WordsFile = m_DictionaryFile + ".words";
    {
        FILE* fp = fopen (WordsFile.c_str(), "w");
        if (!fp) return false;
        for (std::map<std::string,  std::vector<CLexProb> >::const_iterator it = Dictionary.begin(); it != Dictionary.end(); it++)
            fprintf (fp, "%s\n", it->first.c_str());
        fclose (fp);
    }

    std::string SizesFile = m_DictionaryFile + ".sizes";
    {
        std::vector<uint32_t> v;
        for (std::map<std::string,  std::vector<CLexProb> >::const_iterator it = Dictionary.begin(); it != Dictionary.end(); it++)
            v.push_back( (uint32_t)it->second.size() );
        if (!WriteVector(SizesFile, v))
            return false;
    }

    std::string IntersFile = m_DictionaryFile + ".interps";
    {
        FILE* fp = fopen (IntersFile.c_str(), "wb");
        if (!fp) return false;
        for (std::map<std::string,  std::vector<CLexProb> >::const_iterator it = Dictionary.begin(); it != Dictionary.end(); it++)
            if (!WriteVectorInner(fp, it->second))
                return false;
        fclose (fp);
    }
    return true;
}

bool CTrigramModel::read_dictionary_binary()
{
    std::vector<uint32_t> sizes;
    ReadVector(m_DictionaryFile + ".sizes", sizes);
    ReadVector(m_DictionaryFile + ".interps", m_LexProbs);

    std::string WordsFile = m_DictionaryFile + ".words";
    {
        FILE* fp = fopen (WordsFile.c_str(), "r");
        if (!fp) return false;
        char buffer[2*1024];
        size_t no  = 0;
        size_t offset = 0;
        m_Dictionary.clear();
        while (fgets(buffer,  2*1024, fp))
        {
            rtrim (buffer);
            uint32_t size = sizes[no];
            CTrigramWord btw;
            btw.m_Length = size;
            btw.m_StartOffset = offset;
            btw.m_WordStr = buffer;
            
            m_Dictionary.push_back( btw );
            no++;
            offset += size;
        }
        fclose (fp);
        if (no  != sizes.size())
            return false;
        if (offset  != m_LexProbs.size())
            return false;
    }

    return true;
}


bool CTrigramModel::convert_to_lex_binary() 
{

    std::map<std::string,  std::vector<CLexProb> > Dictionary;
    if (!read_dictionary_text_file( Dictionary))
        throw CExpc ("canot read dictionary");

    if (!write_dictionary_binary(Dictionary))
        throw CExpc ("caanot read binary dictionary");

    if (!read_dictionary_binary())
        throw CExpc ("caanot read binary dictionary");

    if (Dictionary.size() != m_Dictionary.size())
        throw CExpc ("Error! Dictionary.,size() != m_Dictionary.size()");

    return true;

}

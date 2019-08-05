//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Tue Jul 17 08:18:29 CEST 2018
// Last Modified: Tue Jul 17 08:18:24 CEST 2018
// Filename:      tool-simat.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-simat.h
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Interface for simat (similarity matrix) tool.
//

#ifndef _TOOL_SIMAT_H_INCLUDED
#define _TOOL_SIMAT_H_INCLUDED

#include "HumTool.h"
#include "HumdrumFile.h"

#include <iostream>

namespace hum {

// START_MERGE

class MeasureData {
	public:
		            MeasureData               (void);
		            MeasureData               (HumdrumFile& infile,
		                                       int startline,int stopline);
		            MeasureData               (HumdrumFile* infile,
		                                      int startline,int stopline);
		           ~MeasureData               ();
		void        setOwner                  (HumdrumFile* infile);
		void        setOwner                  (HumdrumFile& infile);
		void        setStartLine              (int startline);
		void        setStopLine               (int stopline);
		int         getStartLine              (void);
		int         getStopLine               (void);
		void        clear                     (void);
		std::vector<double>& getHistogram7pc (void);
		void        generateNoteHistogram     (void);
		double      getSum7pc                 (void);
		double      getStartTime              (void);
		double      getStopTime               (void);
		double      getDuration               (void);
		int         getMeasure                (void);
		std::string getQoff                   (void);
		std::string getQon                    (void);
		double      getScoreDuration          (void);

	private:
		HumdrumFile*        m_owner       = NULL;
		int                 m_startline   = -1;
		int                 m_stopline    = -1;
		std::vector<double> m_hist7pc;
		double              m_sum7pc      = 0.0;
};



class MeasureDataSet {
	public:
		             MeasureDataSet   (void);
		             MeasureDataSet   (HumdrumFile& infile);
		            ~MeasureDataSet   ();

		void         clear            (void);
		int          parse            (HumdrumFile& infile);
		MeasureData& operator[]       (int index);
		int          size             (void) { return (int)m_data.size(); }
		double       getScoreDuration (void);

	private:
		std::vector<MeasureData*> m_data;
};



class MeasureComparison {
	public:
		MeasureComparison();
		MeasureComparison(MeasureData& data1, MeasureData& data2);
		MeasureComparison(MeasureData* data1, MeasureData* data2);
		~MeasureComparison();

		void clear(void);
		void compare(MeasureData& data1, MeasureData& data2);
		void compare(MeasureData* data1, MeasureData* data2);

		double getCorrelation7pc(void);

	protected:
		double correlation7pc = 0.0;
};



class MeasureComparisonGrid {
	public:
		             MeasureComparisonGrid     (void);
		             MeasureComparisonGrid     (MeasureDataSet& set1, MeasureDataSet& set2);
		             MeasureComparisonGrid     (MeasureDataSet* set1, MeasureDataSet* set2);
		            ~MeasureComparisonGrid     ();

		void         clear                     (void);
		void         analyze                   (MeasureDataSet& set1, MeasureDataSet& set2);
		void         analyze                   (MeasureDataSet* set1, MeasureDataSet* set2);

		double       getStartTime1             (int index);
		double       getStopTime1              (int index);
		double       getDuration1              (int index);
		int          getMeasure1               (int index);
		std::string  getQon1                   (int index);
		std::string  getQoff1                  (int index);
		double       getScoreDuration1         (void);
		double       getStartTime2             (int index);
		double       getStopTime2              (int index);
		double       getDuration2              (int index);
		int          getMeasure2               (int index);
		std::string  getQon2                   (int index);
		std::string  getQoff2                  (int index);
		double       getScoreDuration2         (void);

		ostream&     printCorrelationGrid      (ostream& out = std::cout);
		ostream&     printCorrelationDiagonal  (ostream& out = std::cout);
		ostream&     printSvgGrid              (ostream& out = std::cout);
		void         getColorMapping           (double input, double& hue, double& saturation,
				 double& lightness);

	private:
		std::vector<std::vector<MeasureComparison>> m_grid;
		MeasureDataSet* m_set1 = NULL;
		MeasureDataSet* m_set2 = NULL;
};



class Tool_simat : public HumTool {
	public:
		         Tool_simat         (void);
		        ~Tool_simat         () {};

		bool     run                (HumdrumFileSet& infiles);
		bool     run                (HumdrumFile& infile1, HumdrumFile& infile2);
		bool     run                (const string& indata1, const string& indata2, ostream& out);
		bool     run                (HumdrumFile& infile1, HumdrumFile& infile2, ostream& out);

	protected:
		void     initialize         (HumdrumFile& infile1, HumdrumFile& infile2);
		void     processFile        (HumdrumFile& infile1, HumdrumFile& infile2);

	private:
		MeasureDataSet        m_data1;
		MeasureDataSet        m_data2;
		MeasureComparisonGrid m_grid;

};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_SIMAT_H_INCLUDED */




/**
 * \file XLinkBondMap.h
 * $Revision: 1.00 $
 * \brief Object for representing the potential cross-links for peptides.
 *************************************************************************/
#ifndef XLINKBONDMAP_H
#define XLINKBONDMAP_H

#include "XLinkSite.h"
#include <map>
#include <set>
#include <string>


/**
 * \class XLinkBondMap
 * \brief object representing the potential cross-links for peptides.
 */
class XLinkBondMap: public std::map<XLinkSite, std::set<XLinkSite> > {

 public:

  /**
   * Default constructor.
   */
  XLinkBondMap();

  /**
   * Constructor that initializes XLinkBondMap using the links string.
   * Format: A:B,C:D,... which means that a link can occur between residue
   * A and B, or C and D.
   */
  XLinkBondMap(
    std::string& links_string ///<link string
    );

  /**
   * Initializes XLinkBondMap using the links string.
   * Format: A:B,C:D,... which means that a link can occur between residue
   * A and B, or C and D.
   */
  void init(
    std::string& links_string ///< links string
    );  
  
  /**
   * Default destructor
   */
  virtual ~XLinkBondMap();

  /**
   * \returns whether a cross-link can occur at a single position in the 
   * peptide (for deadlinks).
   */
  bool canLink(
    Crux::Peptide* peptide, ///<peptide object pointer
    int idx             ///<sequence idx
    ); 

  /**
   * \returns whether a cross-link can occur between two positions in the 
   * peptide (for selfloops).
   */
  bool canLink(
    Crux::Peptide* peptide, ///<peptide object pointer
    int idx1,           ///<1st sequence idx
    int idx2            ///<2nd sequence idx
    ); 

  /**
   * \returns whether a cross-link can occur between two peptides at their 
   * respective sequence positions (for inter/intra links).
   */
  bool canLink(
    Crux::Peptide* peptide1,  ///<1st peptide object pointer 
    Crux::Peptide* peptide2,  ///<2nd peptide object pointer
    int idx1,             ///<1st peptide sequence idx
    int idx2              ///<2nd peptide sequence idx
    );
  
};

#endif
/**
 * \file Enzyme.h
 * \brief Decides which pairs of amino acids the digestion enzyme cleaves.
 */

/*
 * AUTHOR: William Stafford Noble
 * CREATE DATE: 19 December 2012
 * $Revision: 1.22 $
 *****************************************************************************/
#ifndef ENZYME_H
#define ENZYME_H

#include <stdio.h>
#include <string>

using namespace Crux;

/**
 * Remove ambiguous characters from a set of amino acids.  Note that
 * the resulting string may contain the same amino acid twice.
 */
void Enzyme::removeAmbiguousAminos(
  string& aminos
) {

  string nonAmbiguousAminos;
  for (myChar = aminos.begin(); myChar < aminos.end(); myChar++) {

    // B = D or N
    if (myChar == "B") {
      nonAmbiguousAminos.append("DN");
    }
    // Z = E or Q
    else if (myChar == "Z") {
      nonAmbiguousAminos.append("EQ");
    }
    // O, J, U = illegal
    else if ((myChar == "O") || (myChar == "U") || (myChar == "J")) {
      carp(CARP_ERROR, "Illegal amino acid (%c) in enzyme rule.\n", myChar);
      exit(1);
    }
    // X = any amino acid
    else if (myChar == "X") {
      nonAmbiguousAminos.append(allAminoAcids_);
    } else {
      nonAmbiguousAminos.append(myChar);
    }
  }

  aminos = nonAmbiguousAminos;
}

/**
 * Take the complement of a set of amino acids.
 */
void Enzyme::complementAminos(
  string& aminos
) {

  string complementAminos;
  for (myChar = allAminos_.begin(); myChar < allAminos_.end(); myChar++) {
    if (aminos.find(myChar) == string::npos) {
      complementAminos.append(myChar);
    }
  }
  aminos = complementAminos
}

void Enzyme::init(
  const char* enzymeName
) {

  // If the enzyme is named, convert it to the corresponding custom string.
  string enzymeRule;
  if (strcmp(enzymeName, "trypsin") == 0){
    enzymeRule = "[KR]|{P}";
  } else if (strcmp(enzymeName, "chymotrypsin") == 0){
    enzymeRule = "[FWY]|{P}";
  } else if (strcmp(enzymeName, "elastase") == 0){
    enzymeRule = "[ALIV]|{P}";
  } else if (strcmp(enzymeName, "clostripain") == 0){
    enzymeRule = "[R]|[]";
  } else if (strcmp(enzymeName, "cyanogen-bromide") == 0){
    enzymeRule = "[M]|[]";
  } else if (strcmp(enzymeName, "iodosobenzoate") == 0){
    enzymeRule = "[W]|[]";
  } else if (strcmp(enzymeName, "proline-endopeptidase") == 0){
    enzymeRule = "[P]|[]";
  } else if (strcmp(enzymeName, "staph-protease") == 0){
    enzymeRule = "[E]|[]";
  } else if (strcmp(enzymeName, "modified-chymotrypsin") == 0){
    enzymeRule = "[FWYL]|{P}";
  } else if (strcmp(enzymeName, "elastase-trypsin-chymotrypsin") == 0){
    enzymeRule = "[ALIVKRWFY]|{P}";
  } else if (strcmp(enzymeName, "aspn") == 0){
    enzymeRule = "[]|[D]";
  } else if (strcmp(enzymeName, "no-enzyme") == 0){
    enzymeRule = "[X]|[X]";
  } else if (strcmp(enzymeName, "no-cleavage") == 0){
    enzymeRule = "[]|[]";
  } else {
    enzymeRule = enzymeName;
  }

  // Find the vertical bar.
  barPosition = -1;
  for (int idx = 0; idx < enzymeRule.len(); idx++) {
    if (enzymeRule[idx] == "|") {
      if (barPosition == -1) {
        barPosition = idx;
      } else {
	carp(CARP_ERROR, "Two vertical bars in enzyme rule (%s).\n",
	     enzymeRule);
	exit(1);
      }
    }
  }
  if (barPosition == -1) {
    carp(CARP_ERROR, "Unrecognized enzyme name (%s).\n", enzymeRule);
    exit(1);
  }

  // Extract the two strings.
  string precedingAminos = enzymeRule.substr(1, barPosition - 2);
  string followingAminos = 
    enzymeRule.substr(barPosition + 1,
		      enzymeRule.length() - barPosition - 2);

  // Remove ambiguous characters.
  removeAmbiguousAminos(&precedingAminos);
  removeAmbiguousAminos(&followingAminos);

  // Complement the two strings, if necessary.
  if ((enzymeRule[0] == "[") && (enzymeRule[barPosition - 1] == "]")) {
    complementAminos(&precedingAminos);
  } else if !((enzymeRule[0] == "{") && (enzymeRule[barPosition - 1] == "}")) {
    carp(CARP_ERROR, "Failure to parse first half of enzyme rule (%s).\n",
	 enzymeRule);
    exit(1);
  }
  if ((enzymeRule[barPosition + 1] == "[") && 
      (enzymeRule[enzymeRule.length() - 1] == "]")) {
    complementAminos(&followingAminos);
  } else if !((enzymeRule[barPosition + 1] == "{") && 
	      (enzymeRule[enzymeRule.length() - 1] == "}")) {
    carp(CARP_ERROR, "Failure to parse second half of enzyme rule (%s).\n",
	 enzymeRule);
    exit(1);
  }

  // Fill in the mappings.
  for (myChar = 'A'; myChar <= 'Z'; myChar++) {
    precedingCleavage_[myChar] = false;
    followingCleavage_[myChar] = false;
  }
  for (myChar = precedingString.begin(); 
       myChar < precedingString.end(); myChar++) {
    precedingCleavage_[myChar] = true;
  }
  for (myChar = followingString.begin(); 
       myChar < followingString.end(); myChar++) {
    followingCleavage_[myChar] = true;
  }

  // FIXME: Cope with ambiguity codes in input.

}

/**
 * \returns An enzyme of the specified type.
 */
Enzyme::Enzyme(
  const char* enzymeName
) {
  return(init(enzymeName));
}

/**  
 * Decides whether the enzyme cleaves between the two specified amino acids.
 * Use 
 */
bool Enzyme::cleaves(
  char preceding,
  char following
) {
  return(precedingCleavage_[preceding] && followingCleavage_[following]);
}

/**
 * Frees an allocated Enzyme object.
 */
Enzyme::~Scorer() {
  // FIXME
}

/*
 * Local Variables:
 * mode: c
 * c-basic-offset: 2
 * End:
 */
#endif
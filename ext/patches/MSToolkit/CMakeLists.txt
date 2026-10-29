cmake_minimum_required(VERSION 3.15)
cmake_policy(SET CMP0091 NEW)
project(mstoolkit)

#############################################################################
# COMPILE mzimltools
#
# Crux builds MSToolkit itself from the copy in the Comet source tree. That
# copy doesn't include the mzIdentML tools or the pepXML writer used by Kojak,
# so only those are built here, into the mzimltools library. They use the
# expat headers from this source tree, but link against the expat built into
# Comet's mstoolkitlite (same expat version).
###############################################################################

set(EXPAT_PATH ./src/expat-2.2.9/lib)
set(MST_PATH ./src/MSToolkit)
set(MZIMLTOOLS_PATH ./src/mzIMLTools)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_GLIBCXX_USE_CXX11_ABI=0")

include_directories(./include ${EXPAT_PATH})

set(
  MZIMLTOOLS
  ${MZIMLTOOLS_PATH}/CAdditionalSearchParams.cpp 
  ${MZIMLTOOLS_PATH}/CAffiliation.cpp
  ${MZIMLTOOLS_PATH}/CAmbiguousResidue.cpp
  ${MZIMLTOOLS_PATH}/CAnalysisCollection.cpp 
  ${MZIMLTOOLS_PATH}/CAnalysisData.cpp
  ${MZIMLTOOLS_PATH}/CAnalysisParams.cpp 
  ${MZIMLTOOLS_PATH}/CAnalysisProtocolCollection.cpp
  ${MZIMLTOOLS_PATH}/CAnalysisSampleCollection.cpp 
  ${MZIMLTOOLS_PATH}/CAnalysisSoftware.cpp 
  ${MZIMLTOOLS_PATH}/CAnalysisSoftwareList.cpp 
  ${MZIMLTOOLS_PATH}/CAuditCollection.cpp 
  ${MZIMLTOOLS_PATH}/CBibliographicReference.cpp 
  ${MZIMLTOOLS_PATH}/CContactRole.cpp 
  ${MZIMLTOOLS_PATH}/CCvList.cpp 
  ${MZIMLTOOLS_PATH}/CDBSequence.cpp
  ${MZIMLTOOLS_PATH}/CDataCollection.cpp 
  ${MZIMLTOOLS_PATH}/CDatabaseFilters.cpp 
  ${MZIMLTOOLS_PATH}/CDatabaseName.cpp 
  ${MZIMLTOOLS_PATH}/CEnzyme.cpp 
  ${MZIMLTOOLS_PATH}/CEnzymeName.cpp 
  ${MZIMLTOOLS_PATH}/CEnzymes.cpp
  ${MZIMLTOOLS_PATH}/CFileFormat.cpp 
  ${MZIMLTOOLS_PATH}/CFilter.cpp 
  ${MZIMLTOOLS_PATH}/CFilterType.cpp 
  ${MZIMLTOOLS_PATH}/CFragmentArray.cpp 
  ${MZIMLTOOLS_PATH}/CFragmentation.cpp 
  ${MZIMLTOOLS_PATH}/CFragmentationTable.cpp 
  ${MZIMLTOOLS_PATH}/CFragmentTolerance.cpp 
  ${MZIMLTOOLS_PATH}/CExclude.cpp 
  ${MZIMLTOOLS_PATH}/CInclude.cpp 
  ${MZIMLTOOLS_PATH}/CInputs.cpp 
  ${MZIMLTOOLS_PATH}/CIonType.cpp
  ${MZIMLTOOLS_PATH}/CMassTable.cpp 
  ${MZIMLTOOLS_PATH}/CMeasure.cpp
  ${MZIMLTOOLS_PATH}/CModification.cpp 
  ${MZIMLTOOLS_PATH}/CModificationParams.cpp 
  ${MZIMLTOOLS_PATH}/CMzIdentML.cpp 
  ${MZIMLTOOLS_PATH}/COrganization.cpp 
  ${MZIMLTOOLS_PATH}/CParent.cpp 
  ${MZIMLTOOLS_PATH}/CParentTolerance.cpp 
  ${MZIMLTOOLS_PATH}/CPeptide.cpp 
  ${MZIMLTOOLS_PATH}/CPeptideEvidence.cpp 
  ${MZIMLTOOLS_PATH}/CPeptideHypothesis.cpp
  ${MZIMLTOOLS_PATH}/CPerson.cpp 
  ${MZIMLTOOLS_PATH}/CProteinAmbiguityGroup.cpp 
  ${MZIMLTOOLS_PATH}/CProteinDetection.cpp 
  ${MZIMLTOOLS_PATH}/CProteinDetectionHypothesis.cpp  
  ${MZIMLTOOLS_PATH}/CProteinDetectionList.cpp 
  ${MZIMLTOOLS_PATH}/CProteinDetectionProtocol.cpp
  ${MZIMLTOOLS_PATH}/CProvider.cpp 
  ${MZIMLTOOLS_PATH}/CPSM.cpp 
  ${MZIMLTOOLS_PATH}/CResidue.cpp 
  ${MZIMLTOOLS_PATH}/CRole.cpp
  ${MZIMLTOOLS_PATH}/CSample.cpp
	${MZIMLTOOLS_PATH}/CSearchDatabase.cpp 
  ${MZIMLTOOLS_PATH}/CSearchModification.cpp 
  ${MZIMLTOOLS_PATH}/CSearchType.cpp 
  ${MZIMLTOOLS_PATH}/CSequenceCollection.cpp 
  ${MZIMLTOOLS_PATH}/CSiteRegexp.cpp 
  ${MZIMLTOOLS_PATH}/CSoftwareName.cpp
  ${MZIMLTOOLS_PATH}/CSourceFile.cpp 
  ${MZIMLTOOLS_PATH}/CSpecificityRules.cpp 
  ${MZIMLTOOLS_PATH}/CSpectraData.cpp 
  ${MZIMLTOOLS_PATH}/CSpectrumIDFormat.cpp
  ${MZIMLTOOLS_PATH}/CSpectrumIdentification.cpp 
  ${MZIMLTOOLS_PATH}/CSpectrumIdentificationItem.cpp
  ${MZIMLTOOLS_PATH}/CSpectrumIdentificationList.cpp 
  ${MZIMLTOOLS_PATH}/CSpectrumIdentificationProtocol.cpp 
  ${MZIMLTOOLS_PATH}/CSpectrumIdentificationResult.cpp
  ${MZIMLTOOLS_PATH}/CSubSample.cpp 
  ${MZIMLTOOLS_PATH}/CThreshold.cpp
)

add_library(
  mzimltools
  STATIC
  ${MZIMLTOOLS}
  ${MST_PATH}/pepXMLWriter.cpp
)

if (UNIX)
  target_compile_options(
    mzimltools
    PRIVATE
    -O3 -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -DGCC
  )
elseif(WIN32 AND NOT CYGWIN)
  target_compile_definitions(
    mzimltools
    PRIVATE
    _CRT_SECURE_NO_WARNINGS
  )
endif(UNIX)

################################################################################

# INSTALL mzimltools. Only the mzIdentML and pepXML headers are installed so
# they don't replace the MSToolkit headers installed by the Comet build.
install(TARGETS mzimltools DESTINATION lib)
install(
  DIRECTORY include/
  DESTINATION include/MSToolkit
  FILES_MATCHING
    PATTERN "C*.h"
    PATTERN "mzIMLStructs.h"
    PATTERN "mzIMLTools.h"
    PATTERN "pepXMLWriter.h"
    # zlib header, matched by "C*.h" on case-insensitive file systems
    PATTERN "crc32.h" EXCLUDE
)

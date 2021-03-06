import FWCore.ParameterSet.Config as cms

from RecoBTag.SecondaryVertex.candidateCombinedSecondaryVertexSoftLeptonCtagLComputer_cfi import *

candidateNegativeCombinedSecondaryVertexSoftLeptonCtagLComputer = candidateCombinedSecondaryVertexSoftLeptonCtagLComputer.clone()
candidateNegativeCombinedSecondaryVertexSoftLeptonCtagLComputer.vertexFlip = True
candidateNegativeCombinedSecondaryVertexSoftLeptonCtagLComputer.trackFlip = True
candidateNegativeCombinedSecondaryVertexSoftLeptonCtagLComputer.trackSelection.sip3dSigMax = 0
candidateNegativeCombinedSecondaryVertexSoftLeptonCtagLComputer.trackPseudoSelection.sip3dSigMax = 0
candidateNegativeCombinedSecondaryVertexSoftLeptonCtagLComputer.trackPseudoSelection.sip2dSigMin = -99999.9
candidateNegativeCombinedSecondaryVertexSoftLeptonCtagLComputer.trackPseudoSelection.sip2dSigMax = -2.0

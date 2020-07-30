""" 
A set of algorithms to calculate new cost types for Graphs  
========================================================================

"""

from .graph import Graph
from .spatial_structures_native_functions import C_CalculateAndStoreCrossSlope, C_CalculateAndStoreEnergyExpenditure

class CostAlgorithmKeys():
    """ Contains the keys for each cost algorithm in Cost Algorithms """
    CROSS_SLOPE = "CrossSlope"
    ENERGY_EXPENDITURE = "EnergyExpenditure"

def CalculateCrossSlope(g : Graph):
    """ Calculate cross slope for every edge in a graph and store it as a
    new cost type.

    Args:
        g (Graph): The graph of nodes and edges to calculate this score for

    Postcondition:
        The graph will be updated with a new cost type containing the results of this algorithm, asccessible with the key in CostAlgorithmKeys.CROSS_SLOPE.
    """
    C_CalculateAndStoreCrossSlope(g.graph_ptr)

def CalculateEnergyExpenditure(g : Graph):
    """ Calculate EnergyExpenditure for every edge in a graph and store it 
    as a new cost type.

    Args:
        g (Graph): The graph of nodes and edges to calculate this algorithm on.

    Postcondition:
        The graph will be updated with a new cost type containing the results of this algorithm, asccessible with the key in CostAlgorithmKeys.ENERGY_EXPENDITURE.
    """
    C_CalculateAndStoreEnergyExpenditure(g.graph_ptr)


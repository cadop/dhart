""" 
A set of algorithms to calculate new cost types for Graphs  
========================================================================

"""

from .graph import Graph


class CostAlgorithmKeys():
    """ Contains the keys for each cost algorithm in Cost Algorithms """
    CROSS_SLOPE = "Cross Slope"
    ENERGY_EXPENDITURE = "Energy Expenditure"

def CalculateCrossSlope(g : Graph):
    """ Calculate cross slope for every edge in a graph and store it as a
    new cost type.

    Args:
        g (Graph): The graph of nodes and edges to calculate this score for

    Postcondition:
        The graph will be updated with a new cost type containing the results of this algorithm, asccessible with the key in CostAlgorithmKeys.ENERGY_EXPENDITURE.
    """
    raise NotImplementedError()

def CalculateEnergyExpenditure(g : Graph):
    """ Calculate EnergyExpenditure for every edge in a graph and store it 
    as a new cost type.

    Args:
        g (Graph): The graph of nodes and edges to calculate this algorithm on.

    Postcondition:
        The graph will be updated with a new cost type containing the results of this algorithm, asccessible with the key in CostAlgorithmKeys.ENERGY_EXPENDITURE.
    """
    raise NotImplementedError()


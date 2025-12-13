#include <VyLib/Graph/VyDAGraph.h>

namespace Vy
{
    VyDAGraph::~VyDAGraph()
    {
        clear();
    }

    void VyDAGraph::clear()
    {
        for(auto& node : m_Nodes) 
        {
            if(node) 
            {
                delete node;
            }
        }

        for(auto& edge : m_Edges)
        {
            if (edge) 
            {
                delete edge;
            }
        }

        m_Nodes.clear();
        m_Edges.clear();
    }


    void VyDAGraph::link(NodePtr from, NodePtr to, EdgePtr edge)
    {
        edge->From = from->getID();
        edge->To   = to  ->getID();

        m_OutEdgesMap[ from->getID() ].insert(edge->getID());
        m_InEdgesMap [ to  ->getID() ].insert(edge->getID());
    }


    void VyDAGraph::remove(NodeID id)
    {
        for(auto& edgeID : m_OutEdgesMap[id]) 
        {
            m_InEdgesMap[ getEdge(edgeID)->To ].erase(edgeID); // Delete outgoing edge

            delete m_Edges[ edgeID ];

            m_Edges[ edgeID ] = nullptr;
        }

        for(auto& edgeID : m_InEdgesMap[id]) 
        {
            m_OutEdgesMap[ getEdge(edgeID)->To ].erase(edgeID); // Delete incoming edge

            delete m_Edges[ edgeID ];

            m_Edges[ edgeID ] = nullptr;
        }

        delete m_Nodes[ id ];
        
        m_Nodes[ id ] = nullptr;

        m_InEdgesMap [ id ].clear();
        m_OutEdgesMap[ id ].clear();
    }
}
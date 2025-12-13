#pragma once

#include <VyLib/Common/Numeric.h>
#include <VyLib/STL/Containers.h>
#include <VyLib/STL/Pointers.h>

namespace Vy
{
    /**
     * @brief Directed acyclic graph.
     * 
     */
    class VyDAGraph
    {
    public:
        using NodeID = U32;
        using EdgeID = U32;

        class Edge;
        // ----------------------------------------------------------------------------------------
        
        class Node
        {
        public:
            virtual ~Node() = default;

            inline NodeID getID() 
            { 
                return ID; 
            }

            template<typename Type = Edge>
            TVector<Type*> getInEdges() 
            { 
                return Graph->getInEdges<Type>(getID()); 
            }

            template<typename Type = Edge>
            TVector<Type*> getOutEdges() 
            { 
                return Graph->getOutEdges<Type>(getID()); 
            }

        private:
            NodeID     ID;
            VyDAGraph* Graph;

            friend class VyDAGraph;
        };

        using NodePtr = Node*;

        // ----------------------------------------------------------------------------------------
        
        class Edge
        {
        public:
            virtual ~Edge() = default;

            inline EdgeID getID() 
            { 
                return ID; 
            }

            template<typename Type = Node>
            inline Type* from() 
            { 
                return Graph->getNode<Type>(From); 
            }

            template<typename Type = Node>
            inline Type* to() 
            { 
                return Graph->getNode<Type>(To); 
            }

        private:
            EdgeID     ID;
            NodeID     From;
            NodeID     To;
            VyDAGraph* Graph;

            friend class VyDAGraph;
        };
        
        using EdgePtr = Edge*;
        
        // ----------------------------------------------------------------------------------------
        
    public:
        VyDAGraph() = default;

        ~VyDAGraph();

        NodeID insert();

        void clear();

        void link(NodePtr from, NodePtr to, EdgePtr edge);
        
        void remove(NodePtr node) 
        { 
            return remove(node->getID());  // When deleting, the associated edges are automatically deleted and the system is destroyed.
        }         
        
        void remove(NodeID id);

        template<typename Type = Node, typename... Args>
        Type* createNode(Args&&... args) 
        {
            Type* node = new Type(std::forward<Args>(args)...);

            node->ID    = m_Nodes.size();
            node->Graph = this;

            m_Nodes.push_back(node);

            m_OutEdgesMap[ node->ID ] = {};
            m_InEdgesMap [ node->ID ] = {};

            return node;
        }

        template<typename Type = Edge, typename... Args>
        Type* createEdge(Args&&... args) 
        {
            Type* edge = new Type(std::forward<Args>(args)...);

            edge->ID    = m_Edges.size();
            edge->Graph = this;
            
            m_Edges.push_back(edge);

            return edge;
        }

        template<typename Type = Node>
        inline Type* getNode(NodeID id) 
        { 
            return dynamic_cast<Type*>(m_Nodes[id]); 
        }

        template<typename Type = Node>
        inline TVector<Type*> getNodes() 
        { 
            TVector<Type*> castNodes;

            for(auto& node : m_Nodes)
            {
                Type* castNode =  dynamic_cast<Type*>(node);

                if (castNode != nullptr)
                {
                    castNodes.push_back(castNode);
                }
            }

            return castNodes;
        }

        template<typename Type = Edge>
        inline Type* getEdge(EdgeID id) 
        { 
            return dynamic_cast<Type*>(m_Edges[id]); 
        };

        template<typename Type = Edge>
        inline TVector<Type*> getEdges() 
        { 
            TVector<Type*> castEdges;

            for(auto& edge : m_Edges)
            {
                Type* castEdge =  dynamic_cast<Type*>(edge);

                if (castEdge != nullptr)
                {
                    castEdges.push_back(castEdge);
                }
            }

            return castEdges;
        }

        template<typename Type = Edge>
        TVector<Type*> getOutEdges(NodeID id)
        {
            TVector<Type*> edges;

            for(auto& edgeID : m_OutEdgesMap[id]) 
            { 
                Type* edge = dynamic_cast<Type*>(getEdge(edgeID));

                if(edge) 
                {
                    edges.push_back(edge); 
                }
            }

            return  edges;
        }

        template<typename Type = Edge>
        TVector<Type*> getInEdges(NodeID id)
        {
            TVector<Type*> edges;

            for(auto& edgeID : m_InEdgesMap[id]) 
            { 
                Type* edge = dynamic_cast<Type*>(getEdge(edgeID));

                if(edge) 
                {
                    edges.push_back(edge); 
                }
            }

            return  edges;
        }

        template<typename Type = Edge>
        TVector<Type*> getOutEdges(NodePtr node)     
        { 
            return getOutEdges<Type>(node->getID()); 
        }

        template<typename Type = Edge>
        TVector<Type*> getInEdges(NodePtr node)      
        { 
            return getInEdges<Type>(node->getID()); 
        } 

    private:
        TVector<EdgePtr> m_Edges; // list of all vertices and edges, which may be empty after removal.
        TVector<NodePtr> m_Nodes;

        THashMap<NodeID, TSet<EdgeID>> m_OutEdgesMap;
        THashMap<NodeID, TSet<EdgeID>> m_InEdgesMap;
    };

    using VyDAGraphRef = std::shared_ptr<VyDAGraph>;
}
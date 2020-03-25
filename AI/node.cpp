#include "node.hpp"
#include "node.hpp"
#include "Logic/logic.h"
#include "ai.h"
#include <ctime>
#include <random>

AI* Node::s_ai = nullptr;
double Node::s_cste = sqrt(2);

Node::Node(AI& ai):
	m_actionId(0), m_threadId(0), m_depth(0), m_value(VAL_INIT), 
	m_nbSimu(0)
{
	s_ai = &ai;
	std::srand(std::time(nullptr));
}

Node::Node(const size_t& actionId_, const unsigned char& threadId_, const unsigned short& depth_):
	m_actionId(actionId_), m_threadId(threadId_), m_depth(depth_), m_value(VAL_INIT),
	m_nbSimu(0)
{
}

size_t Node::actionId() const
{
	return m_actionId;
}

unsigned char Node::threadId() const
{
	return m_threadId;
}

unsigned short Node::depth() const
{
	return m_depth;
}

unsigned short Node::value() const
{
	return m_value;
}

unsigned int Node::nbSimu() const
{
	//if root node
	if (isRoot())
	{
		return s_ai->_getRootNbSimu();
	}
	else
	{
		return m_nbSimu;
	}
}

double Node::getMCTSValue(const unsigned int parentNbSimu) const
{
	return (10 * static_cast<double>(USHRT_MAX-m_value))/USHRT_MAX + s_cste * sqrt(log(parentNbSimu)/m_nbSimu);
}


void Node::pushCrtAction(const size_t& actionId, const unsigned char threadIdx)
{
	auto it = m_sons.find(actionId);
	if (it != m_sons.end())
	{
		//std::cout << "[DEBUG] action " << actionId << " will be handled by thread " << static_cast<unsigned int>(threadIdx) << std::endl;
		it->second->setThreadIdx(threadIdx);
	}
	else
	{
		//std::cout << "[DEBUG] new action " << actionId << " will be handled by thread " << static_cast<unsigned int>(threadIdx) << std::endl;
		m_sons[actionId] = std::make_unique<Node>(actionId, threadIdx, m_depth + 1);
	}
}

void Node::setThreadIdx(const unsigned char threadIdx)
{
	m_threadId = threadIdx;
	for (auto& subNode : m_sons)
	{
		subNode.second->setThreadIdx(threadIdx);
	}
}

bool Node::isRoot() const
{
	return m_threadId==0;
}

unsigned short Node::makeSimu()
{
	//make simulation
	m_value = makeSimu(m_threadId, m_actionId, m_depth);

	//increment nb simulation
	m_nbSimu++;

	return m_value;
}

unsigned short Node::makeSimu(const size_t& threadIdx, const size_t& action, const unsigned short depth)
{
	unsigned int val = VAL_INIT;

	//play move
	N_Logic::Logic::apply(threadIdx, action);

	//check if node is terminal
	if (N_Logic::Logic::isAlreadyPlayed(threadIdx) || !N_Logic::Logic::canBeDemonstrated(threadIdx))
	{
		val = USHRT_MAX;
	}
	else if (N_Logic::Logic::isDemonstrated(threadIdx))
	{
		//std::cout << "[DEBUG] found demonstration!" << std::endl;
		val = 0;
	}
	else if (depth >= MAX_DEPTH)
	{
		val = VAL_INIT;
		//std::cout << "[DEBUG ]Reach limit" << std::endl;
	}
	else
	{
		if (!s_ai->mustStop(threadIdx))
		{	
			//get actions
			std::vector<size_t> actions = N_Logic::Logic::getActions(threadIdx);

			//play random action
			auto newAction = actions[rand() % actions.size()];
			unsigned short val = makeSimu(threadIdx, newAction,depth+1);
			if (val < VAL_INIT)
			{
				val++;
			}			
		}
	}

	//unplay crt move
	N_Logic::Logic::unapply(threadIdx);

	return val;
}

unsigned short Node::explore(const std::vector<size_t>& actions)
{	
	//std::cout << "Explore Node: "<<m_actionId<<" at depth="<<m_depth << std::endl;
	unsigned int retValue = 0;
	double maxValue = 0;
	std::vector<Node*> maxNodes;
	for (size_t k=0;k<actions.size();k++)
	{
		auto action = actions[k];
		auto it = m_sons.find(action);
		//if the node has not been created yet, create it!
		if (it == m_sons.end())
		{
			m_sons[action] = std::make_unique<Node>(action, m_threadId, m_depth + 1);
			it = m_sons.find(action);
		}
		auto subNode = it->second.get();
		//if subNode have already been explored
		if (subNode->nbSimu())
		{
			if (maxValue == std::numeric_limits<double>::max())
			{
				continue;
			}

			double mctValue = subNode->getMCTSValue(nbSimu());
			if (mctValue > maxValue)
			{
				maxNodes.clear();
				maxNodes.push_back(subNode);
				maxValue = mctValue;
			}
			else if (mctValue == maxValue)
			{
				maxNodes.push_back(subNode);
			}
		}
		//if subNode have not been explored
		else
		{
			if (maxValue < std::numeric_limits<double>::max())
			{
				maxValue = std::numeric_limits<double>::max();
				maxNodes.clear();
			}
			maxNodes.push_back(subNode);
		}
	}

	if (isRoot())
	{
		//Choose among maxNodes which one to explore
		auto nbNodes = maxNodes.size();
		auto node = maxNodes[rand() % nbNodes];
		if (node->nbSimu())
		{
			node->explore();
		}
		else
		{
			node->makeSimu();
		}
		s_ai->_incrRootNbSimu();
	}
	else if (!s_ai->mustStop(m_threadId))
	{
		//Choose among maxNodes which one to explore
		auto nbNodes = maxNodes.size();
		auto node = maxNodes[rand() % nbNodes];
		if (node->nbSimu())
		{
			retValue = node->explore();
			if ((m_value > retValue + 1))
			{
				m_value = retValue + 1;
			}
		}
		else
		{
			m_value = node->makeSimu();
		}

			
		m_nbSimu++;
	}		

	return m_value;
}

/**
* if explore method is called, subNodes have already been created
* return value of selected subNode
*/
unsigned short Node::explore()
{
	//play crt move
	N_Logic::Logic::apply(m_threadId, m_actionId);

	//check if node is terminal
	if (N_Logic::Logic::isAlreadyPlayed(m_threadId) || !N_Logic::Logic::canBeDemonstrated(m_threadId))
	{
		m_value = USHRT_MAX;
		//increment nb simulations
		m_nbSimu++;
	}
	else if(N_Logic::Logic::isDemonstrated(m_threadId))
	{
		m_value = 0;
		//increment nb simulations
		m_nbSimu++;
	}
	else
	{
		//get actions
		std::vector<size_t> actions = N_Logic::Logic::getActions(m_threadId);
		
		//explore one subNode among actions
		explore(actions);		
	}

	//unplay crt move
	N_Logic::Logic::unapply(m_threadId);	

	return m_value;
}

Node* Node::getBestNode()
{
	std::vector<size_t> minNodes;
	unsigned short minVal = USHRT_MAX;
	for (auto& son : m_sons)
	{
		if (son.second->value() < minVal)
		{
			minVal = son.second->value();
			minNodes.clear();
			minNodes.push_back(son.first);
		}
		else if (son.second->value() == minVal)
		{
			minNodes.push_back(son.first);
		}
	}
	auto valWinner= minNodes[rand() % minNodes.size()];
	auto winner = m_sons[valWinner].release();
	m_sons.clear();
	std::cout << "[DEBUG] Nb simulations "<<m_nbSimu << std::endl;
	//m_sons[winner.first] = nullptr; //dereference winner in sons
	return winner;
}

void Node::setRoot()
{
	m_threadId = 0;
	_decrDepth();
}

unsigned int Node::getRootNbSimu() const
{
	return m_nbSimu;
}

void Node::incrRootNbSimu()
{
	m_nbSimu++;
}

void Node::_incrNbSimu()
{
	if (isRoot())
	{
		s_ai->_incrRootNbSimu();
	}
	else
	{
		m_nbSimu++;
	}
}

void Node::_decrDepth()
{

	m_depth--;
	for (auto& son : m_sons)
	{
		son.second->_decrDepth();
	}
}

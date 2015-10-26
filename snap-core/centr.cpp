namespace TSnap {

/////////////////////////////////////////////////
// Node centrality measures
double GetDegreeCentr(const PUNGraph& Graph, const int& NId) {
  if (Graph->GetNodes() > 1) {
    return double(Graph->GetNI(NId).GetDeg())/double(Graph->GetNodes()-1); }
  else { return 0.0; }
}

void GetEigenVectorCentr(const PUNGraph& Graph, TIntFltH& NIdEigenH, const double& Eps, const int& MaxIter) {
  const int NNodes = Graph->GetNodes();
  NIdEigenH.Gen(NNodes);
  // initialize vector values
  for (TUNGraph::TNodeI NI = Graph->BegNI(); NI < Graph->EndNI(); NI++) {
    NIdEigenH.AddDat(NI.GetId(), 1.0/NNodes);
    IAssert(NI.GetId() == NIdEigenH.GetKey(NIdEigenH.Len()-1));
  }
  TFltV TmpV(NNodes);
  for (int iter = 0; iter < MaxIter; iter++) {
    int j = 0;
    // add neighbor values
    for (TUNGraph::TNodeI NI = Graph->BegNI(); NI < Graph->EndNI(); NI++, j++) {
      TmpV[j] = 0;
      for (int e = 0; e < NI.GetOutDeg(); e++) {
        TmpV[j] += NIdEigenH.GetDat(NI.GetOutNId(e)); }
    }

    // normalize
    double sum = 0;
    for (int i = 0; i < TmpV.Len(); i++) {
      sum += (TmpV[i]*TmpV[i]);
    }
    sum = sqrt(sum);
    for (int i = 0; i < TmpV.Len(); i++) {
      TmpV[i] /= sum;
    }

    // compute difference
    double diff = 0.0;
    j = 0;
    for (TUNGraph::TNodeI NI = Graph->BegNI(); NI < Graph->EndNI(); NI++, j++) {
      diff += fabs(NIdEigenH.GetDat(NI.GetId())-TmpV[j]);
    }

    // set new values
    j = 0;
    for (TUNGraph::TNodeI NI = Graph->BegNI(); NI < Graph->EndNI(); NI++, j++) {
      NIdEigenH.AddDat(NI.GetId(), TmpV[j]);
    }

    if (diff < Eps) {
      break;
    }
  }
}

// Group centrality measures
double GetGroupDegreeCentr(const PUNGraph& Graph, const PUNGraph& Group) {
  int deg;
  TIntH NN;
  for (TUNGraph::TNodeI NI = Group->BegNI(); NI < Group->EndNI(); NI++) { 
    deg = Graph->GetNI(NI.GetId()).GetDeg();
    for (int i=0; i<deg; i++) {
      if (Group->IsNode(Graph->GetNI(NI.GetId()).GetNbrNId(i))==0)
      NN.AddDat(Graph->GetNI(NI.GetId()).GetNbrNId(i),NI.GetId());
    }
  }
  return (double)NN.Len();
}

double GetGroupDegreeCentr0(const PUNGraph& Graph, const TIntH& GroupNodes) {
  int deg;
  TIntH NN;
  for (int i = 0; i<GroupNodes.Len(); i++) { 
    deg = Graph->GetNI(GroupNodes.GetDat(i)).GetDeg();
    for (int j = 0; j < deg; j++) {
      if (GroupNodes.IsKey(Graph->GetNI(GroupNodes.GetDat(i)).GetNbrNId(j))==0)
      NN.AddDat(Graph->GetNI(GroupNodes.GetDat(i)).GetNbrNId(j),GroupNodes.GetDat(i));
    }
  }
  return (double)NN.Len();
}

double GetGroupDegreeCentr(const PUNGraph& Graph, const TIntH& GroupNodes) {
  int deg;
  TIntH NN;
  TIntH GroupNodes1;

  for (THashKeyDatI<TInt,TInt> NI = GroupNodes.BegI(); NI < GroupNodes.EndI(); NI++)
    GroupNodes1.AddDat(NI.GetDat(),NI.GetDat());

  for (THashKeyDatI<TInt,TInt> NI = GroupNodes1.BegI(); NI < GroupNodes1.EndI(); NI++){
    TUNGraph::TNodeI node = Graph->GetNI(NI.GetKey());
    deg = node.GetDeg();
    for (int j = 0; j < deg; j++){
      if (GroupNodes1.IsKey(node.GetNbrNId(j))==0 && NN.IsKey(node.GetNbrNId(j))==0)
        NN.AddDat(node.GetNbrNId(j),NI.GetKey());
    }
  }

  return (double)NN.Len();
}

double GetGroupFarnessCentr(const PUNGraph& Graph, const TIntH& GroupNodes) {
  TIntH* NDistH = new TIntH[GroupNodes.Len()];
  
  for (int i=0; i<GroupNodes.Len(); i++){
    NDistH[i](Graph->GetNodes());
  TSnap::GetShortPath<PUNGraph>(Graph, GroupNodes.GetDat(i), NDistH[i], true, TInt::Mx);
  }

  int min, dist, sum=0, len=0;
  for (PUNGraph::TObj::TNodeI NI = Graph->BegNI(); NI < Graph->EndNI(); NI++){
    if(NDistH[0].IsKey(NI.GetId()))
      min = NDistH[0].GetDat(NI.GetId());
    else
      min = -1;
    for (int j=1; j<GroupNodes.Len(); j++){
    if (NDistH[j].IsKey(NI.GetId()))
      dist = NDistH[j].GetDat(NI.GetId());
      else
      dist = -1;
    if ((dist < min && dist != -1) || (dist > min && min == -1))
      min = dist;
    }
    if (min>0){  
      sum += min;
      len++;
  }
    
  }

  if (len > 0) { return sum/double(len); }
  else { return 0.0; }
}

PUNGraph *AllGraphsWithNNodes(int n){
  PUNGraph* g = new PUNGraph[(((n*n)-n)/2)+1];
  PUNGraph g0;
  for(int i=0; i<n; i++)
    g0->AddNode(i);
  
  g[0] = g0;
  int br=1;

  for(int i=0; i<n; i++)
  for(int j=i; j<n; j++){
      g0->AddEdge(i,j);
    g[br] = g0;
    br++;
  }

  return g;
}

TIntH *AllCombinationsMN(int m, int n){
  float N = 1;
  for(int i=n; i>0; i--){
    N *= (float)m/(float)n;
    m--;
  n--;
  }

  TIntH* C = new TIntH[(int)N];
  return C;
}

double GetGroupClosenessCentr(const PUNGraph& Graph, const TIntH& GroupNodes) {
  const double Farness = GetGroupFarnessCentr(Graph, GroupNodes);
  if (Farness != 0.0) { return 1.0/Farness; }
  else { return 0.0; }
}

TIntH MaxCPGreedyBetter(const PUNGraph& Graph, const int k) {
  TIntH GroupNodes; // buildup cpntainer of group nodes
  TIntH NNodes; // container of neighbouring nodes
  TIntH Nodes; // nodes sorted by vd
  double gc = 0, gc0 = 0;
  int addId = 0, addIdPrev = 0;
  
  for (TUNGraph::TNodeI NI = Graph->BegNI(); NI < Graph->EndNI(); NI++) {
    Nodes.AddDat(NI.GetId(),NI.GetDeg());
  }

  Nodes.SortByDat(false);

  int br = 0;
  while (br < k) {
    for (THashKeyDatI<TInt,TInt> NI = Nodes.BegI(); NI < Nodes.EndI(); NI++) {
      if ((NI.GetDat() <= (int)gc0))
        break;
      gc = NI.GetDat()-Intersect(Graph->GetNI(NI.GetKey()),NNodes);
      if (gc>gc0) {
        gc0 = gc;
        addId = NI.GetKey();
      }
    }
  
    if (addId != addIdPrev){

      GroupNodes.AddDat(br,addId);
      br++;
      gc0=0;

      NNodes.AddDat(addId,0);
      for (int i=0; i<Graph->GetNI(addId).GetDeg(); i++) {
        NNodes.AddDat(Graph->GetNI(addId).GetNbrNId(i),0);
      }
      addIdPrev = addId;
      Nodes.DelKey(addId);
    } else {
      br = k;
    }
    printf("%i,",br);
  }

  // gcFinal = GetGroupDegreeCentr(Graph, GroupNodes);
  return GroupNodes;
}

// this is the variation of the first version that doesent stop after finding the optimal K
TIntH MaxCPGreedyBetter1(const PUNGraph& Graph, const int k) {
  TIntH GroupNodes;
  TIntH NNodes;
  TIntH Nodes;
  double gc = 0, gc0 = 0;
  int addId = 0, addIdPrev = 0;
  
  // put nodes in the container and sort them by vertex degree
  for (TUNGraph::TNodeI NI = Graph->BegNI(); NI < Graph->EndNI(); NI++){
    Nodes.AddDat(NI.GetId(),NI.GetDeg());
  }
  Nodes.SortByDat(false);

  int br = 0;
  while (br < k) {
    for (THashKeyDatI<TInt,TInt> NI = Nodes.BegI(); NI < Nodes.EndI(); NI++){
      if((NI.GetDat() < (int)gc0))
        break;
      gc = NI.GetDat()-Intersect(Graph->GetNI(NI.GetKey()),NNodes);
      if (gc>gc0) {
        gc0 = gc;
        addId = NI.GetKey();
      }
    }
  
    if (addId != addIdPrev){

      GroupNodes.AddDat(br,addId);
      br++;
      gc0=-10000000;
  
      NNodes.AddDat(addId,0);
      for (int i=0; i<Graph->GetNI(addId).GetDeg(); i++) {
        NNodes.AddDat(Graph->GetNI(addId).GetNbrNId(i),0);
      }
      addIdPrev = addId;
      Nodes.DelKey(addId);
    }
  }

  // gcFinal = GetGroupDegreeCentr(Graph, GroupNodes);
  return GroupNodes;
}

// version with string type of container of group nodes - Fail (it is slower)
TIntH MaxCPGreedyBetter2(const PUNGraph& Graph, const int k) {
  TIntH GroupNodes; // buildup cpntainer of group nodes
  TStr NNodes; // container of neighbouring nodes
  TIntH Nodes; // nodes sorted by vd
  double gc = 0, gc0 = 0;
  int addId = 0, addIdPrev=0;
  
  for (TUNGraph::TNodeI NI = Graph->BegNI(); NI < Graph->EndNI(); NI++){
    Nodes.AddDat(NI.GetId(),NI.GetDeg());
  }

  Nodes.SortByDat(false);

  int br=0;
  while (br < k) {
    for (THashKeyDatI<TInt,TInt> NI = Nodes.BegI(); NI < Nodes.EndI(); NI++){
      if((NI.GetDat() <= (int)gc0))
        break;
      gc = NI.GetDat()-Intersect(Graph->GetNI(NI.GetKey()),NNodes);
      if (gc>gc0) {
        gc0 = gc;
        addId = NI.GetKey();
      }
    }
  
    if (addId != addIdPrev) {

      GroupNodes.AddDat(br,addId);
      br++;
      gc0=0;
    
      TInt digi = addId;
      TStr buf = digi.GetStr();

      NNodes += " "+buf;

      for (int i=0; i<Graph->GetNI(addId).GetDeg(); i++) {
        TInt digi = Graph->GetNI(addId).GetNbrNId(i);
        TStr buf = digi.GetStr();
        NNodes += " "+buf;
      }
      addIdPrev = addId;
      Nodes.DelKey(addId);
    } else {
      br = k;
    }
    printf("%i,",br);
  }

  // gcFinal = GetGroupDegreeCentr(Graph, GroupNodes);
  return GroupNodes;
}

// version with int array - the fastest
TIntH MaxCPGreedyBetter3(const PUNGraph& Graph, const int k) {
  TIntH GroupNodes; // buildup cpntainer of group nodes
  const int n = Graph->GetNodes();
  int *NNodes = new int[n]; // container of neighbouring nodes
  int NNodes_br = 0;
  TIntH Nodes; // nodes sorted by vd
  double gc = 0, gc0 = 0;
  int addId = 0, addIdPrev = 0;
  
  for (TUNGraph::TNodeI NI = Graph->BegNI(); NI < Graph->EndNI(); NI++){
    Nodes.AddDat(NI.GetId(),NI.GetDeg());
  }

  Nodes.SortByDat(false);

  int br = 0;
  while (br < k) {
    for (THashKeyDatI<TInt,TInt> NI = Nodes.BegI(); NI < Nodes.EndI(); NI++){
      if((NI.GetDat() <= (int)gc0))
        break;
      gc = NI.GetDat()-Intersect(Graph->GetNI(NI.GetKey()),NNodes,NNodes_br);
      if (gc>gc0){
        gc0 = gc;
        addId = NI.GetKey();
      }
    }
  
    if (addId != addIdPrev) {

      GroupNodes.AddDat(br,addId);
      br++;
      gc0=0;

      int nn = addId;
      bool nnnew = true;
      for (int j=0; j<NNodes_br; j++)
        if (NNodes[j] == nn){
          nnnew = false;
          j = NNodes_br;
        }
  
      if (nnnew){
        NNodes[NNodes_br] = nn;
        NNodes_br++;
      }

      for (int i=0; i<Graph->GetNI(addId).GetDeg(); i++) {
        int nn = Graph->GetNI(addId).GetNbrNId(i);
        bool nnnew = true;
        for (int j=0; j<NNodes_br; j++) {
          if (NNodes[j] == nn){
            nnnew = false;
            j = NNodes_br;
          }
        }
        if (nnnew){
          NNodes[NNodes_br] = nn;
          NNodes_br++;
        }
      }
      addIdPrev = addId;
      Nodes.DelKey(addId);
    } else {
      br = k;
    }
    printf("%i,",br);
  }

  delete NNodes;
  // gcFinal = GetGroupDegreeCentr(Graph, GroupNodes);
  return GroupNodes;
}

//Weighted PageRank
int GetWeightedPageRank(const PNEANet Graph, TIntFltH& PRankH, const TStr& Attr, const double& C, const double& Eps, const int& MaxIter) {
  if (!Graph->IsFltAttrE(Attr)) return -1;

  TFltV Weights = Graph->GetFltAttrVecE(Attr);

  int mxid = Graph->GetMxNId();
  TFltV OutWeights(mxid);
  Graph->GetWeightOutEdgesV(OutWeights, Weights);
  /*for (TNEANet::TNodeI NI = Graph->BegNI(); NI < Graph->EndNI(); NI++) {
    OutWeights[NI.GetId()] = Graph->GetWeightOutEdges(NI, Attr);
  }*/


  /*TIntFltH Weights;
  for (TNEANet::TNodeI NI = Graph->BegNI(); NI < Graph->EndNI(); NI++) {
    Weights.AddDat(NI.GetId(), Graph->GetWeightOutEdges(NI, Attr));
  }*/

  const int NNodes = Graph->GetNodes();
  //const double OneOver = 1.0/double(NNodes);
  PRankH.Gen(NNodes);
  for (TNEANet::TNodeI NI = Graph->BegNI(); NI < Graph->EndNI(); NI++) {
    PRankH.AddDat(NI.GetId(), 1.0/NNodes);
    //IAssert(NI.GetId() == PRankH.GetKey(PRankH.Len()-1));
  }
  TFltV TmpV(NNodes);
  for (int iter = 0; iter < MaxIter; iter++) {
    int j = 0;
    for (TNEANet::TNodeI NI = Graph->BegNI(); NI < Graph->EndNI(); NI++, j++) {
      TmpV[j] = 0;
      for (int e = 0; e < NI.GetInDeg(); e++) {
        const int InNId = NI.GetInNId(e);
        const TFlt OutWeight = OutWeights[InNId];
        int EId = Graph->GetEId(InNId, NI.GetId());
        const TFlt Weight = Weights[Graph->GetFltKeyIdE(EId)];
        if (OutWeight > 0) {
          TmpV[j] += PRankH.GetDat(InNId) * Weight / OutWeight; }
      }
      TmpV[j] =  C*TmpV[j]; // Berkhin (the correct way of doing it)
      //TmpV[j] =  C*TmpV[j] + (1.0-C)*OneOver; // iGraph
    }
    double diff=0, sum=0, NewVal;
    for (int i = 0; i < TmpV.Len(); i++) { sum += TmpV[i]; }
    const double Leaked = (1.0-sum) / double(NNodes);
    for (int i = 0; i < PRankH.Len(); i++) { // re-instert leaked PageRank
      NewVal = TmpV[i] + Leaked; // Berkhin
      //NewVal = TmpV[i] / sum;  // iGraph
      diff += fabs(NewVal-PRankH[i]);
      PRankH[i] = NewVal;
    }
    if (diff < Eps) { break; }
  }
  return 0;
}

#ifdef USE_OPENMP

int GetWeightedPageRankMP1(const PNEANet Graph, TIntFltH& PRankH, const TStr& Attr, const double& C, const double& Eps, const int& MaxIter) {
  if (!Graph->IsFltAttrE(Attr)) return -1;

  TFltV Weights = Graph->GetFltAttrVecE(Attr);

  int mxid = Graph->GetMxNId();
  TFltV OutWeights(mxid);
  Graph->GetWeightOutEdgesV(OutWeights, Weights);
  /*for (TNEANet::TNodeI NI = Graph->BegNI(); NI < Graph->EndNI(); NI++) {
    OutWeights[NI.GetId()] = Graph->GetWeightOutEdges(NI, Attr);
  }*/


  /*TIntFltH Weights;
  for (TNEANet::TNodeI NI = Graph->BegNI(); NI < Graph->EndNI(); NI++) {
    Weights.AddDat(NI.GetId(), Graph->GetWeightOutEdges(NI, Attr));
  }*/

  const int NNodes = Graph->GetNodes();
  TVec<TNEANet::TNodeI> NV;
  //const double OneOver = 1.0/double(NNodes);
  PRankH.Gen(NNodes);
  for (TNEANet::TNodeI NI = Graph->BegNI(); NI < Graph->EndNI(); NI++) {
    NV.Add(NI);
    PRankH.AddDat(NI.GetId(), 1.0/NNodes);
    //IAssert(NI.GetId() == PRankH.GetKey(PRankH.Len()-1));
  }
  TFltV TmpV(NNodes);
  for (int iter = 0; iter < MaxIter; iter++) {
    #pragma omp parallel for schedule(dynamic,10000)
    for (int j = 0; j < NNodes; j++) {
      TNEANet::TNodeI NI = NV[j];
      TmpV[j] = 0;
      for (int e = 0; e < NI.GetInDeg(); e++) {
        const int InNId = NI.GetInNId(e);
        const TFlt OutWeight = OutWeights[InNId];
        int EId = Graph->GetEId(InNId, NI.GetId());
        const TFlt Weight = Weights[Graph->GetFltKeyIdE(EId)];
        if (OutWeight > 0) {
          TmpV[j] += PRankH.GetDat(InNId) * Weight / OutWeight; }
      }
      TmpV[j] =  C*TmpV[j]; // Berkhin (the correct way of doing it)
      //TmpV[j] =  C*TmpV[j] + (1.0-C)*OneOver; // iGraph
    }
    double diff=0, sum=0, NewVal;
    #pragma omp parallel for reduction(+:sum) schedule(dynamic,10000)
    for (int i = 0; i < TmpV.Len(); i++) { sum += TmpV[i]; }
    const double Leaked = (1.0-sum) / double(NNodes);
    #pragma omp parallel for reduction(+:diff) schedule(dynamic,10000)
    for (int i = 0; i < PRankH.Len(); i++) { // re-instert leaked PageRank
      NewVal = TmpV[i] + Leaked; // Berkhin
      //NewVal = TmpV[i] / sum;  // iGraph
      diff += fabs(NewVal-PRankH[i]);
      PRankH[i] = NewVal;
    }
    if (diff < Eps) { break; }
  }
  return 0;
}

int GetWeightedPageRankMP2(const PNEANet Graph, TIntFltH& PRankH, const TStr& Attr, const double& C, const double& Eps, const int& MaxIter) {
  if (!Graph->IsFltAttrE(Attr)) return -1;
  const int NNodes = Graph->GetNodes();
  TVec<TNEANet::TNodeI> NV;

  //const double OneOver = 1.0/double(NNodes);
  PRankH.Gen(NNodes);
  int MxId = 0;

  for (TNEANet::TNodeI NI = Graph->BegNI(); NI < Graph->EndNI(); NI++) {
    NV.Add(NI);
    PRankH.AddDat(NI.GetId(), 1.0/NNodes);
    int Id = NI.GetId();
    if (Id > MxId) {
      MxId = Id;
    }
  }

  TFltV PRankV(MxId+1);
  TFltV OutWeights(MxId+1);

  TFltV Weights = Graph->GetFltAttrVecE(Attr);

  #pragma omp parallel for schedule(dynamic,10000)
  for (int j = 0; j < NNodes; j++) {
    TNEANet::TNodeI NI = NV[j];
    int Id = NI.GetId();
    OutWeights[Id] = Graph->GetWeightOutEdges(NI, Attr);
    PRankV[Id] = 1/NNodes;
  }

  TFltV TmpV(NNodes);
  for (int iter = 0; iter < MaxIter; iter++) {

    #pragma omp parallel for schedule(dynamic,10000)
    for (int j = 0; j < NNodes; j++) {
      TNEANet::TNodeI NI = NV[j];
      TFlt Tmp = 0;
      for (int e = 0; e < NI.GetInDeg(); e++) {
        const int InNId = NI.GetInNId(e);

        const TFlt OutWeight = OutWeights[InNId];

        int EId = Graph->GetEId(InNId, NI.GetId());
        const TFlt Weight = Weights[Graph->GetFltKeyIdE(EId)];

        if (OutWeight > 0) {
          Tmp += PRankH.GetDat(InNId) * Weight / OutWeight;
        }
      }
      TmpV[j] =  C*Tmp; // Berkhin (the correct way of doing it)
      //TmpV[j] =  C*TmpV[j] + (1.0-C)*OneOver; // iGraph
    }

    double sum = 0;
    #pragma omp parallel for reduction(+:sum) schedule(dynamic,10000)
    for (int i = 0; i < TmpV.Len(); i++) { sum += TmpV[i]; }
    const double Leaked = (1.0-sum) / double(NNodes);

    double diff = 0;
    #pragma omp parallel for reduction(+:diff) schedule(dynamic,10000)
    for (int i = 0; i < NNodes; i++) {
      TNEANet::TNodeI NI = NV[i];
      double NewVal = TmpV[i] + Leaked; // Berkhin
      //NewVal = TmpV[i] / sum;  // iGraph
      int Id = NI.GetId();
      diff += fabs(NewVal-PRankV[Id]);
      PRankV[Id] = NewVal;
    }
    if (diff < Eps) { break; }
  }

  #pragma omp parallel for schedule(dynamic,10000)
  for (int i = 0; i < NNodes; i++) {
    TNEANet::TNodeI NI = NV[i];
    PRankH[i] = PRankV[NI.GetId()];
  }

  return 0;
}

#endif // USE_OPENMP

//Event importance
TIntFltH EventImportance(const PNGraph& Graph, const int k) {
  TIntFltH NodeList; // values for nodese

  for (TNGraph::TNodeI NI = Graph->BegNI(); NI < Graph->EndNI(); NI++){
    NodeList.AddDat(NI.GetId(),NI.GetOutDeg());
  }


  for (THashKeyDatI<TInt,TFlt> NI = NodeList.BegI(); NI < NodeList.EndI(); NI++){
    int outdeg = Graph->GetNI(NI.GetKey()).GetOutDeg();
    int indeg = Graph->GetNI(NI.GetKey()).GetInDeg();
    
    if (outdeg>1 && indeg>0){
      double val = (1-(1/(double)outdeg))/(double)indeg;
      for(int i=0; i<(outdeg+indeg);i++){
        int nid = Graph->GetNI(NI.GetKey()).GetNbrNId(i);
        if (Graph->GetNI(NI.GetKey()).IsInNId(nid) == true){
        NodeList.AddDat(nid,NodeList.GetDat(nid)+val);
        }
        
      }
    }
    
  }

  return NodeList;
}

//Event importance 1
TIntFltH EventImportance1 (const PNGraph& Graph, const int k) {
  TIntFltH NodeList; // values for nodese

  for (TNGraph::TNodeI NI = Graph->BegNI(); NI < Graph->EndNI(); NI++){
    NodeList.AddDat(NI.GetId(),NI.GetOutDeg());
  }


  for (THashKeyDatI<TInt,TFlt> NI = NodeList.BegI(); NI < NodeList.EndI(); NI++){
    int outdeg = Graph->GetNI(NI.GetKey()).GetOutDeg();
    int indeg = Graph->GetNI(NI.GetKey()).GetInDeg();
    
    if (outdeg>1 && indeg>0){
      double val = (1-(1/(double)outdeg))/(double)indeg;
      for(int i=0; i<(outdeg+indeg);i++){
        int nid = Graph->GetNI(NI.GetKey()).GetNbrNId(i);
        if (Graph->GetNI(NI.GetKey()).IsInNId(nid) == true){
        NodeList.AddDat(nid,NodeList.GetDat(nid)+val);
        }
        
      }
    }
    
  }

  return NodeList;
}

int Intersect(TUNGraph::TNodeI Node, TIntH NNodes){
  int br=0;
  for (int i=0; i<Node.GetDeg(); i++)
  {
    if (NNodes.IsKey(Node.GetNbrNId(i)))
      br++;
  }
  if (NNodes.IsKey(Node.GetId()))
    br++;

  return br;
}

int Intersect(TUNGraph::TNodeI Node, TStr NNodes){
  int br=0;

  TInt digi = -1;
  TStr buf = "";

  for (int i=0; i<Node.GetDeg(); i++)
  {
    digi = Node.GetNbrNId(i);
    TStr buf = digi.GetStr();

    if (NNodes.IsStrIn(buf.CStr()))
    br++;
  }

  digi = Node.GetId();
  buf = digi.GetStr();

  if (NNodes.IsStrIn(buf.CStr()))
    br++;

  return br;
}

int Intersect(TUNGraph::TNodeI Node, int *NNodes, int NNodes_br){
  int br = 0;
  int neig;
  for (int i=0; i<Node.GetDeg(); i++)
  {
    neig = Node.GetNbrNId(i);
    for (int j=0; j<NNodes_br; j++)
    {
    if (neig == NNodes[j])
    {
      br++;
      j = NNodes_br;
    }
    }
  }

  neig = Node.GetId();
  for (int j=0; j<NNodes_br; j++)
  {
    if (neig == NNodes[j])
    {
      br++;
      j = NNodes_br;
    }
  }

  return br;
}

int Intersect1(TUNGraph::TNodeI Node, TStr NNodes){
  int br=0;
  for (int i=0; i<Node.GetDeg(); i++)
  {
    TInt digi = Node.GetNbrNId(i);
    TStr buf = "";
    buf = digi.GetStr();

    if (NNodes.SearchStr(buf.CStr())!=-1)
    br++;
  }
  
  TInt digi = Node.GetId();
  TStr buf = digi.GetStr();

  if (NNodes.SearchStr(buf.CStr())!=-1)
    br++;

  return br;
}

TIntH LoadNodeList(TStr InFNmNodes){
  TSsParser Ss(InFNmNodes, ssfWhiteSep, true, true, true);
  TIntIntH Nodes;
  int br = 0, NId;
  while (Ss.Next()) {
    if (Ss.GetInt(0, NId)) { 
    Nodes.AddDat(br,NId);
    br++;
  }
  }
  return Nodes;
}


}; // namespace TSnap

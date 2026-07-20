void inspect() {
  TFile *f = TFile::Open("./test1000.root");
  if (!f || f->IsZombie()) {
    cout << "Error opening file" << endl;
    return;
  }
  cout << "Keys in file:" << endl;
  f->GetListOfKeys()->Print();
  // Look for any TTree
  TIter next(f->GetListOfKeys());
  TKey *key;
  while ((key = (TKey*)next())) {
    TObject *obj = key->ReadObj();
    if (obj->IsA()->InheritsFrom(TTree::Class())) {
      TTree *t = (TTree*)obj;
      cout << "Found TTree: " << t->GetName() << endl;
      t->Print();
      t->Scan();
    }
  }
  f->Close();
}

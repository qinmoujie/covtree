#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

struct fileCovInfo {
  fileCovInfo(const string &file, size_t FNF, size_t FNH, size_t LF, size_t LH)
      : file(file), FNF(FNF), FNH(FNH), LF(LF), LH(LH) {}
  string file;
  size_t FNF;
  size_t FNH;
  size_t LF;
  size_t LH;
};

bool parse_coverage_info_file(const std::string &filename,
                              vector<fileCovInfo> &coverageinfo) {
  ifstream inf(filename);
  if (!inf) {
    cerr << filename << " not exists!!!" << endl;
    return false;
  }
  string buf;
  string file;
  size_t FNF, FNH, LF, LH;
  for (; getline(inf, buf);) {
    if (buf.substr(0, 3) == "SF:") {
      file = buf.substr(3);
      for (; getline(inf, buf);) {
        if (buf.substr(0, 4) == "FNF:") {
          FNF = stoll(buf.substr(4));
          for (; getline(inf, buf);) {
            if (buf.substr(0, 4) == "FNH:") {
              FNH = stoll(buf.substr(4));
              for (; getline(inf, buf);) {
                if (buf.substr(0, 3) == "LF:") {
                  LF = stoll(buf.substr(3));
                  for (; getline(inf, buf);) {
                    if (buf.substr(0, 3) == "LH:") {
                      LH = stoll(buf.substr(3));
                      coverageinfo.emplace_back(file, FNF, FNH, LF, LH);
                      break;
                    }
                  }
                  break;
                }
              }
              break;
            }
          }
          break;
        }
      }
    }
  }
  return true;
}

struct treenode {
  typedef treenode *ptr_node_type;
  treenode(const string &file, size_t FNF, size_t FNH, size_t LF, size_t LH,
           bool leaf)
      : filenode(file, FNF, FNH, LF, LH), leaf(leaf) {}
  fileCovInfo filenode;
  vector<ptr_node_type> child;
  bool leaf;
};

class Dirtree {
 public:
  typedef treenode::ptr_node_type ptr_node_type;
  Dirtree(const string &project, const vector<fileCovInfo> &coverageinfo,
          const string &absCovpath)
      : project(project), absCovpath(absCovpath) {
    for (size_t i = 0; i != coverageinfo.size(); ++i) {
      ptr_node_type cur = root;
      vector<string> nodefile = getnodefile(coverageinfo[i].file);
      for (size_t j = 0; j != nodefile.size(); ++j) {
        const string &fd = nodefile[j];
        if (root == nullptr) {
          root = new treenode(fd, 0, 0, 0, 0, false);
          cur = root;
        } else if (cur->filenode.file != fd) {
          bool flag = false;
          for (const ptr_node_type &ptr : cur->child) {
            if (ptr->filenode.file == fd) {
              flag = true;
              cur = ptr;
              break;
            }
          }
          if (flag == false) {
            if (j == nodefile.size() - 1) {
              auto chd =
                  new treenode(fd, coverageinfo[i].FNF, coverageinfo[i].FNH,
                               coverageinfo[i].LF, coverageinfo[i].LH, true);
              cur->child.push_back(chd);
              cur = chd;
            } else {
              auto chd = new treenode(fd, 0, 0, 0, 0, false);
              cur->child.push_back(chd);
              cur = chd;
            }
          }
        }
      }
    }
    build_tree(root);
  }

  ~Dirtree() { destory_tree(root); }

  void print() { print(root, 0); }

  bool genhtml(const string htmlfile) {
    ofstream ouf(htmlfile);
    if (!ouf) {
      cerr << "error<" << htmlfile << " can't open to write" << endl;
      return false;
    }
    ouf.setf(ios::fixed);
    ouf << setprecision(1);
    ouf << "<body>\n\t<div class=\"tree\">\n\t\t<ul>" << endl;
    ptr_node_type start_root = root;
    string par = "";
    for (; start_root->child.size() == 1;) {
      par.append("/").append(start_root->filenode.file);
      start_root = start_root->child.front();
    }
    genhtml_imple(ouf, start_root, par, 3);
    ouf << "\t\t</ul>\n\t</div>" << endl;
    ouf << "<script>$(\"ul>li\").not(\":eq(0)\").hide()</script>" << endl;
    ouf << "</body>" << endl;
    return true;
  }

 private:
  void genhtml_imple(ofstream &ouf, const ptr_node_type cur, string parpath,
                     size_t dep) {
    if (parpath == project) parpath = "";
    float funcrate = cur->filenode.FNF == 0
                         ? 0
                         : float(cur->filenode.FNH) / cur->filenode.FNF;
    float linerate =
        cur->filenode.LF == 0 ? 0 : float(cur->filenode.LH) / cur->filenode.LF;
    string status = "badge badge-important";
    if (linerate >= 0.9)
      status = "badge badge-success";
    else if (linerate >= 0.7)
      status = "badge badge-warning";
    if (cur->leaf == false) {
      ouf << string(dep, '\t') << "<li>\n"
          << string(dep + 1, '\t') << "<span class=\"" << status
          << "\"><i class=\"icon-plus-sign\"></i> " << cur->filenode.file << " "
          << linerate * 100 << "% " << funcrate * 100 << "% "
          << "</span>\n"
          << string(dep + 1, '\t') << "<ul>" << endl;
      for (ptr_node_type chd : cur->child) {
        genhtml_imple(ouf, chd, parpath + "/" + cur->filenode.file, dep + 1);
      }
      ouf << string(dep + 1, '\t') << "</ul>\n"
          << string(dep, '\t') << "</li>" << endl;
    } else {
      ouf << string(dep + 1, '\t') << "<li>\n"
          << string(dep + 2, '\t') << "<span class=\"" << status << "\"> "
          << linerate * 100 << "% " << funcrate * 100
          << "% </span> &ndash; <a href=\""
          << absCovpath + parpath + "/" + cur->filenode.file + ".gcov.html"
          << "\"> " << cur->filenode.file << "</a>\n"
          << string(dep + 1, '\t') << "</li>" << endl;
    }
  }

  void print(const ptr_node_type cur, size_t dep) {
    for (size_t i = 0; i != dep; ++i) putchar('\t');
    cout << cur->filenode.file << "|" << cur->filenode.LH << "/"
         << cur->filenode.LF << "|" << cur->filenode.FNH << "/"
         << cur->filenode.FNF << "|" << endl;
    for (const ptr_node_type chd : cur->child) print(chd, dep + 1);
  }

  void destory_tree(ptr_node_type cur) {
    for (const ptr_node_type chd : cur->child) destory_tree(chd);
    delete cur;
  }

  vector<string> getnodefile(const string &filepath) {
    vector<string> result;
    size_t start_pos = 0;
    for (size_t pos = 0; pos != string::npos;) {
      start_pos = pos + 1;
      pos = filepath.find('/', start_pos);
      result.push_back(filepath.substr(start_pos, pos - start_pos));
    }
    return result;
  }

  const fileCovInfo &build_tree(ptr_node_type cur) {
    if (cur->leaf == false) {
      for (ptr_node_type chd : cur->child) {
        const fileCovInfo &filecov = build_tree(chd);
        cur->filenode.FNF += filecov.FNF;
        cur->filenode.FNH += filecov.FNH;
        cur->filenode.LF += filecov.LF;
        cur->filenode.LH += filecov.LH;
      }
    }
    return cur->filenode;
  }

 private:
  ptr_node_type root;
  string absCovpath;
  string project;
  string ignore;
};

int main(int argc, char *args[]) {
  vector<fileCovInfo> coverageinfo;
  if (parse_coverage_info_file(args[2], coverageinfo) == false) return 1;
  Dirtree dtree(args[1], coverageinfo, args[3]);
  if (dtree.genhtml(args[4]) == false) return 1;
  return 0;
}
import sys
import re

assert len(sys.argv) == 3, "Usage: %s <src_log> <path>" % sys.argv[0]
src_log = sys.argv[1]
path = sys.argv[2]
assert path.startswith("src")


def path_startswith(child, parent):
    """Check whether one path starts with another.

    Treats path elements as atomic pieces, eg "Makerules.env"
    does not start with "Makerules".
    """
    if not len(child) or child[-1] != "/":
        child += "/"
    if not len(parent) or parent[-1] != "/":
        parent += "/"
    if not len(child) or child[0] != "/":
        child = "/" + child
    if not len(parent) or parent[0] != "/":
        parent = "/" + parent
    return child.startswith(parent)

srcs = eval(open(src_log).read())

best_src = ""
best_dest = ""
for dest, src in srcs:
    if path_startswith(path, dest):
        if len(dest) >= len(best_dest):
            best_dest = dest
            best_src = src

if best_src.startswith("/"):
    m = re.search("/([^/]*(_BRANCH_|_REL_|_TWIG_)[^/]*)/", best_src)
    if m:
        best_src_tag = m.group(1)
    else:
        assert "/trunk/" in best_src
        best_src_tag = "trunk"
else:
    best_src_tag = "LOCAL_COMPONENT"

print best_src_tag

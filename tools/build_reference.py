#!/usr/bin/env python3
"""
build_reference.py -- turn the repo into printable per-category documents.

Emits, into reference/ :
    NN-<slug>.md      portable markdown, fenced ```cpp blocks, anchor TOC
    NN-<slug>.html    self-contained, syntax coloured, print CSS
    index.html        cover page linking all of them
    README.md         the same list, in markdown

To make PDFs: open a .html in Chrome, Ctrl+P, Destination "Save as PDF".
Turn ON "Background graphics" (for the coloured boxes) and, if you want page
numbers, "Headers and footers". Margins: Default.

Re-run this after adding files -- anything not named in ORDER is appended to
its category automatically, sorted, so new templates never go missing.
"""

import html
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OUT = os.path.join(ROOT, "reference")

# ---------------------------------------------------------------------------
# Files deliberately left out: the second copy of an algorithm the repo keeps
# twice. Delete a line here to put one back -- nothing else needs to change.
# The three suffix-array files are NOT here: they differ in character
# (clean implementation / worked examples / implementation + prose), and both
# are wanted at the table.
# ---------------------------------------------------------------------------
DROP = {
    "graph/SCC/Tarjan SCC.cpp",        # strict subset of tarjan - bridges.cpp
    "graph/flows/mcmf (2).cpp",        # second MCMF
    "graph/flows/altdinic.cpp",        # alternate Dinic
    "DS/centroid/i don't know.cpp",    # alternate centroid decomposition
    "strings/Z manacher.cpp",          # alternate Manacher
}

# ---------------------------------------------------------------------------
# Categories, in print order. Each is (slug, title, blurb, [roots], [ORDER]).
# ORDER fixes the sequence of the files that matter; everything else in the
# roots is appended sorted.
# ---------------------------------------------------------------------------
CATEGORIES = [
    ("subproblems", "Subproblems Index",
     "Read this when you have a statement and no plan. Left column is what the "
     "statement says; right column is where to go.",
     ["subproblems"],
     ["subproblems/README.md", "subproblems/graph.md", "subproblems/ds.md",
      "subproblems/strings.md", "subproblems/dp.md", "subproblems/math.md",
      "subproblems/combinatorics.md", "subproblems/number_theory.md",
      "subproblems/geometry.md", "subproblems/game.md"]),

    ("graph", "Graph",
     "Shortest paths, connectivity, flows, matching, spanning trees.",
     ["graph"],
     ["graph/shortest path/dijkstra.cpp", "graph/shortest path/01 bfs.cpp",
      "graph/shortest path/bellman ford.cpp", "graph/shortest path/floyd warshall.cpp",
      "graph/shortest path/dag paths.cpp", "graph/shortest path/k_shortest_paths.cpp",
      "graph/segment_tree_graph.cpp", "graph/min_mean_cycle.cpp",
      "graph/SCC/tarjan - bridges.cpp", "graph/SCC/block_cut_tree.cpp",
      "graph/SCC/two_sat.cpp", "graph/SCC/eulerian_path.cpp",
      "graph/dsu/dsu parity.cpp", "graph/dsu/dsu_rollback.cpp",
      "graph/dsu/dsu_marking.cpp",
      "graph/MST/kruskal with dsu.cpp", "graph/MST/prim mst.cpp",
      "graph/MST/second_best_mst.cpp", "graph/MST/kruskal_tree.cpp",
      "graph/MST/boruvka.cpp",
      "graph/flows/Dinics.cpp", "graph/flows/MCMF.cpp",
      "graph/flows/edmonds karp O(VEE).cpp", "graph/flows/lower_bounds.cpp",
      "graph/flows/stoer_wagner.cpp",
      "graph/matching/kuhn.cpp", "graph/matching/hopcroft_karp.cpp",
      "graph/matching/hungarian.cpp", "graph/matching/gale_shapley.cpp",
      "graph/steiner_tree.cpp", "graph/kirchhoff.cpp", "graph/graph_coloring.cpp",
      "graph/inverse_graph.cpp", "graph/path_intersection.cpp"]),

    ("ds", "Data Structures",
     "Range queries, trees, offline techniques.",
     ["DS"],
     ["DS/segment tree/segment tree.cpp", "DS/segment tree/segtree_iterative.cpp",
      "DS/segment tree/lazy.cpp", "DS/segment tree/beats.cpp",
      "DS/segment tree/implicit.cpp", "DS/segment tree/segtree_2d.cpp",
      "DS/segment tree/persistent/persistent.cpp",
      "DS/segment tree/persistent/persistent count distinct.cpp",
      "DS/segment tree/merge/merge sort tree.cpp",
      "DS/segment tree/merge/merge sort fenwick.cpp",
      "DS/segment tree/merge/segtree_merge.cpp",
      "DS/BIT/pointxrange.cpp", "DS/BIT/rangexrange.cpp", "DS/BIT/fenwick_kth.cpp",
      "DS/BIT/fenwick_2d.cpp", "DS/BIT/fenwick_2d_range.cpp",
      "DS/BIT/fenwick_2d_offline.cpp",
      "DS/sparse/Sparse Table.cpp", "DS/sparse/disjoint_sparse_table.cpp",
      "DS/sparse/sparse_table_2d.cpp",
      "DS/LCA/binary lifting.cpp", "DS/LCA/virtual_tree.cpp",
      "DS/HLD/HLD NODE.cpp", "DS/HLD/HLD edge.cpp",
      "DS/centroid/centroid.cpp", "DS/dsu_on_tree.cpp",
      "DS/SQRT/SQRT.cpp", "DS/SQRT/mo.cpp", "DS/SQRT/mo_on_tree.cpp",
      "DS/SQRT/mo_variants.cpp",
      "DS/offline/cdq.cpp", "DS/offline/parallel_binary_search.cpp",
      "DS/Trie/binary_trie.cpp", "DS/Trie/string_trie.cpp",
      "DS/Trie/persistent_binary_trie.cpp",
      "DS/monotonic_stack.cpp", "DS/monotonic_deque.cpp", "DS/cartesian_tree.cpp",
      "DS/swag.cpp", "DS/odt.cpp", "DS/wavelet_tree.cpp", "DS/bitset.cpp",
      "DS/prefix_sum_2d.cpp", "DS/2d partial sum.cpp",
      "DS/bst/treap_implicit.cpp"]),

    ("dp", "Dynamic Programming",
     "State design first, optimisation second.",
     ["DP"],
     ["DP/lis.cpp", "DP/incexc.cpp", "DP/sos.cpp", "DP/subset_sum_bitset.cpp",
      "DP/submask_partition.cpp", "DP/subset_sum_convolution.cpp",
      "DP/bitmask_tsp.cpp", "DP/broken_profile_dp.cpp",
      "DP/digit dp.cpp", "DP/digit_dp_automaton.cpp",
      "DP/rerooting.cpp", "DP/tree_knapsack.cpp",
      "DP/sub-problems xorbasis.cpp",
      "DP/optimization/CHT.cpp", "DP/optimization/li_chao.cpp",
      "DP/optimization/D&C.cpp", "DP/optimization/knuth.cpp",
      "DP/slope_trick.cpp",
      "DP/matrix/matrix expo.cpp", "DP/matrix/minlenK.cpp"]),

    ("strings", "Strings and Hashing",
     "Matching, suffix structures, palindromes, hashing.",
     ["strings", "hashing"],
     ["strings/Z kmp.cpp", "strings/aho_corasick.cpp", "strings/aho_fail_tree.cpp",
      "strings/trie.cpp", "strings/subsequence_automaton.cpp",
      "strings/suffix_automaton.cpp", "strings/cleansuf.cpp",
      "strings/subsuf.cpp", "strings/suffix array.cpp",
      "strings/separate manacher.cpp", "strings/eertree.cpp", "strings/lyndon.cpp",
      "hashing/hash.cpp", "hashing/hash-vec.cpp", "hashing/hash_seg.cpp",
      "hashing/hash_tree.cpp", "hashing/hash_ms.cpp", "hashing/hash_2d.cpp"]),

    ("math", "Math and Number Theory",
     "Modular arithmetic, linear algebra, polynomials, number theory.",
     ["math", "number theory"],
     ["math/modular.cpp", "math/extgcd.cpp", "math/crt.cpp", "math/phi.cpp",
      "math/bsgs.cpp", "number theory/sieve.cpp", "math/nt_misc.cpp",
      "math/floor_sum.cpp",
      "math/gauss.cpp", "math/interpolation.cpp", "math/linear_recurrence.cpp",
      "math/fft_ntt.cpp", "math/convolution_anymod.cpp", "math/fwht.cpp",
      "math/bitwise.cpp", "math/randomization.cpp", "math/compare_fractions.cpp",
      "math/notes.cpp"]),

    ("combinatorics", "Combinatorics",
     "Counting, symmetry, determinant formulas.",
     ["combinatorics"],
     ["combinatorics/nCr_with_mod.cpp", "combinatorics/nCr_without_mod.cpp",
      "combinatorics/catalan.cpp", "combinatorics/stirling.cpp",
      "combinatorics/burnside.cpp", "combinatorics/counting_formulas.cpp"]),

    ("game", "Game Theory",
     "Impartial games, Grundy values, games with cycles.",
     ["game"],
     ["game/grundy.cpp", "game/retrograde.cpp"]),

    ("misc", "Template, Snippets and Testing",
     "The boilerplate everything pastes into, plus the stress-test generators.",
     ["template", "snippet", "stress"],
     ["template/template.cpp", "snippet/ordered_set.cpp", "snippet/compress.cpp",
      "stress/gen.cpp"]),
]

# ---------------------------------------------------------------------------
# C++ tokenizer -- highlighting is done HERE, at build time, so the HTML is
# static. No JS, nothing to fail during print rendering.
# ---------------------------------------------------------------------------
KEYWORDS = {
    "alignas", "alignof", "and", "asm", "auto", "bool", "break", "case", "catch",
    "char", "class", "const", "constexpr", "continue", "decltype", "default",
    "delete", "do", "double", "else", "enum", "explicit", "export", "extern",
    "false", "float", "for", "friend", "goto", "if", "inline", "int", "long",
    "mutable", "namespace", "new", "noexcept", "not", "nullptr", "operator", "or",
    "private", "protected", "public", "register", "return", "short", "signed",
    "sizeof", "static", "struct", "switch", "template", "this", "throw", "true",
    "try", "typedef", "typename", "union", "unsigned", "using", "virtual", "void",
    "volatile", "while", "xor",
}
TYPES = {
    "array", "bitset", "deque", "greater", "less", "ll", "map", "multiset", "pair",
    "priority_queue", "queue", "set", "size_t", "stack", "string", "tuple",
    "unordered_map", "unordered_set", "vector", "__int128", "int64_t", "uint64_t",
    "mt19937", "mt19937_64", "function",
}

TOKEN_RE = re.compile(r"""
    (?P<comment_block>/\*.*?\*/)
  | (?P<comment_line>//[^\n]*)
  | (?P<preproc>^[ \t]*\#[^\n]*)
  | (?P<string>"(?:\\.|[^"\\])*")
  | (?P<char>'(?:\\.|[^'\\])*')
  | (?P<number>\b(?:0[xX][0-9a-fA-F]+|\d+\.?\d*(?:[eE][+-]?\d+)?)(?:[uUlLfF]*)\b)
  | (?P<word>[A-Za-z_]\w*)
  | (?P<other>.)
""", re.S | re.M | re.X)

# a comment line that is part of a #### warning box in the house style
BOXRE = re.compile(r"^\s*//\s*#")


def highlight_cpp(src: str) -> str:
    """Return HTML with <span class=...> spans. Input is raw C++."""
    out = []
    for m in TOKEN_RE.finditer(src):
        kind = m.lastgroup
        text = m.group()
        esc = html.escape(text)
        if kind in ("comment_line", "comment_block"):
            # split so each line of a box comment can be marked
            parts = esc.split("\n")
            rendered = []
            for p in parts:
                raw = html.unescape(p)
                cls = "cbox" if BOXRE.match(raw) else "c"
                rendered.append(f'<span class="{cls}">{p}</span>' if p else "")
            out.append("\n".join(rendered))
        elif kind == "preproc":
            out.append(f'<span class="pp">{esc}</span>')
        elif kind == "string":
            out.append(f'<span class="s">{esc}</span>')
        elif kind == "char":
            out.append(f'<span class="s">{esc}</span>')
        elif kind == "number":
            out.append(f'<span class="n">{esc}</span>')
        elif kind == "word":
            if text in KEYWORDS:
                out.append(f'<span class="k">{esc}</span>')
            elif text in TYPES:
                out.append(f'<span class="t">{esc}</span>')
            else:
                out.append(esc)
        else:
            out.append(esc)
    return "".join(out)


# ---------------------------------------------------------------------------
# helpers
# ---------------------------------------------------------------------------
def slugify(s: str) -> str:
    """Match GitHub's heading-anchor algorithm, so the TOC links in the .md
    files work when the repo is browsed on GitHub: lowercase, drop everything
    that is not a word character / space / hyphen (underscores SURVIVE), then
    spaces to hyphens."""
    s = s.lower()
    s = re.sub(r"[^\w\s-]", "", s)      # keeps [A-Za-z0-9_]
    s = re.sub(r"\s+", "-", s.strip())
    return s


def nice_name(path: str) -> str:
    """Human title for a file: 'graph/shortest path/01 bfs.cpp' -> '01 bfs'."""
    base = os.path.basename(path)
    return os.path.splitext(base)[0]


def group_of(path: str, roots) -> str:
    """The sub-heading a file sits under, e.g. 'shortest path'."""
    rel = path.replace("\\", "/")
    parts = rel.split("/")
    if len(parts) >= 3:
        return "/".join(parts[1:-1])
    return ""


def collect(cat):
    slug, title, blurb, roots, order = cat
    found = []
    for root in roots:
        d = os.path.join(ROOT, root)
        if not os.path.isdir(d):
            continue
        for dirpath, _dirnames, filenames in os.walk(d):
            for fn in sorted(filenames):
                if not (fn.endswith(".cpp") or fn.endswith(".md")):
                    continue
                full = os.path.join(dirpath, fn)
                rel = os.path.relpath(full, ROOT).replace("\\", "/")
                if rel in DROP:
                    continue
                found.append(rel)
    seen, ordered = set(), []
    for p in order:
        if p in found and p not in seen:
            ordered.append(p)
            seen.add(p)
    for p in sorted(found):
        if p not in seen:
            ordered.append(p)          # anything new shows up automatically
            seen.add(p)
    return ordered


def read(path):
    with open(os.path.join(ROOT, path), "r", encoding="utf-8", errors="replace") as f:
        return f.read()


def anchors_for(files):
    """path -> anchor, deduplicated the way GitHub does it (-1, -2, ...).
    Two files can share a base name across sub-folders (HLD NODE / HLD edge is
    fine, but 'hash' style collisions are not), and a silent collision would
    send both TOC links to the same place."""
    used, m = {}, {}
    for p in files:
        base = slugify(nice_name(p))
        n = used.get(base, 0)
        used[base] = n + 1
        m[p] = base if n == 0 else f"{base}-{n}"
    return m


# ---------------------------------------------------------------------------
# markdown emitter
# ---------------------------------------------------------------------------
def emit_md(cat, files, idx, anch):
    slug, title, blurb, roots, _order = cat
    L = [f"# {idx}. {title}", "", blurb, "",
         f"*{len(files)} files. Generated by `tools/build_reference.py` — do not edit by hand.*",
         "", "---", "", "## Contents", ""]

    last_group = None
    for p in files:
        g = group_of(p, roots)
        if g != last_group:
            if g:
                L.append(f"**{g}**")
                L.append("")
            last_group = g
        L.append(f"- [{nice_name(p)}](#{anch[p]}) — `{p}`")
    L += ["", "---", ""]

    for p in files:
        name = nice_name(p)
        L.append(f"## {name}")
        L.append("")
        L.append(f"`{p}`")
        L.append("")
        body = read(p)
        if p.endswith(".md"):
            # already markdown: demote its headings so they nest under ours
            for line in body.splitlines():
                L.append(("##" + line) if line.startswith("#") else line)
        else:
            L.append("```cpp")
            L.append(body.rstrip("\n"))
            L.append("```")
        L.append("")
        L.append("---")
        L.append("")
    return "\n".join(L) + "\n"


# ---------------------------------------------------------------------------
# HTML emitter
# ---------------------------------------------------------------------------
CSS = r"""
/* ---------------------------------------------------------------------------
   LIGHT ONLY, DELIBERATELY. These documents exist to become paper, so they
   commit to ink-on-white and do not follow the reader's OS theme -- what you
   see on screen is what comes out of the printer. Every colour is stated
   explicitly rather than inherited, so the page holds on any host background.

   The syntax palette is chosen to survive GREYSCALE printing: hue and weight
   both carry, so keywords stay bold-dark and comments stay light-mid even
   with no colour at all.
   --------------------------------------------------------------------------- */
:root{
  color-scheme: light;              /* stop the browser dark-inverting forms */
  --paper:#ffffff; --ink:#16181c; --soft:#5a6470; --faint:#8b939d;
  --rule:#d5dade; --rule2:#aeb6bd;
  --accent:#0d5f6b;
  --c:#4f7a5e;        /* comment            */
  --cbox:#8a5a12;     /* #### box comment   */
  --k:#1f4f9c;        /* keyword            */
  --t:#0d6b6b;        /* type               */
  --s:#8a4520;        /* string             */
  --n:#8a3d6b;        /* number             */
  --pp:#6a3d9c;       /* preprocessor       */
  --codebg:#fafbfb;
}

*{box-sizing:border-box}
html{background:var(--paper)}
body{
  margin:0; background:var(--paper); color:var(--ink);
  font-family:system-ui,-apple-system,"Segoe UI",Roboto,Arial,sans-serif;
  font-size:10.5pt; line-height:1.5;
}
.wrap{max-width:52rem;margin:0 auto;padding:2rem 1.5rem 4rem}

h1{font-size:1.9rem;line-height:1.15;margin:0 0 .35rem;letter-spacing:-.01em}
.blurb{color:var(--soft);margin:0 0 .3rem;max-width:52ch}
.meta{color:var(--faint);font-size:.85em;margin:0}
hr{border:none;border-top:3px double var(--rule2);margin:1.6rem 0}

h2{
  font-family:ui-monospace,"Cascadia Mono",Consolas,Menlo,monospace;
  font-size:1.02rem;margin:0 0 .1rem;letter-spacing:-.01em;
  color:var(--accent);
  break-after:avoid; page-break-after:avoid;
}
.path{
  font-family:ui-monospace,"Cascadia Mono",Consolas,Menlo,monospace;
  font-size:.78em;color:var(--faint);margin:0 0 .45rem;
  break-after:avoid; page-break-after:avoid;
}
.file{ margin:0 0 1.5rem; break-inside:auto; }

/* ---- table of contents ---- */
.toc{margin:0 0 1rem}
.toc h2{color:var(--ink);font-size:1.05rem;margin-bottom:.5rem}
.toc .grp{
  font-family:ui-monospace,"Cascadia Mono",Consolas,Menlo,monospace;
  font-size:.72rem;letter-spacing:.12em;text-transform:uppercase;
  color:var(--faint);margin:.75rem 0 .2rem;
}
.toc ol{list-style:none;margin:0;padding:0;
  columns:2; column-gap:2rem;}
.toc li{break-inside:avoid;margin:.08rem 0;font-size:.92em}
.toc a{color:var(--ink);text-decoration:none;
  display:flex;justify-content:space-between;gap:.5rem;align-items:baseline}
.toc a:hover{color:var(--accent)}
.toc .num{color:var(--faint);font-size:.85em;
  font-family:ui-monospace,"Cascadia Mono",Consolas,Menlo,monospace;
  font-variant-numeric:tabular-nums}

/* ---- code ---- */
pre{
  margin:0; background:var(--codebg);
  border:1px solid var(--rule); border-radius:3px;
  padding:.6rem .75rem;
  font-family:ui-monospace,"Cascadia Mono","DejaVu Sans Mono",Consolas,Menlo,monospace;
  font-size:8.6pt; line-height:1.38;
  white-space:pre-wrap;
  overflow-wrap:break-word;
  padding-left:2.2em; text-indent:-1.6em;   /* hanging indent on wrapped lines */
  tab-size:4;
}
code{font-family:inherit}
.c{color:var(--c)}
.cbox{color:var(--cbox);font-weight:600}
.k{color:var(--k);font-weight:600}
.t{color:var(--t)}
.s{color:var(--s)}
.n{color:var(--n)}
.pp{color:var(--pp)}

/* ---- prose blocks (the already-markdown index files) ---- */
.prose{font-size:.95em}
.prose table{border-collapse:collapse;width:100%;margin:.5rem 0;font-size:.88em}
.prose th,.prose td{border:1px solid var(--rule);padding:.28rem .5rem;
  text-align:left;vertical-align:top}
.prose th{background:var(--codebg);font-weight:650}
.prose h3{font-size:1rem;margin:1rem 0 .3rem;color:var(--accent);
  break-after:avoid;page-break-after:avoid}
.prose h4{font-size:.92rem;margin:.8rem 0 .25rem;
  break-after:avoid;page-break-after:avoid}
.prose code{background:var(--codebg);padding:.05em .3em;border-radius:2px;
  font-size:.9em;border:1px solid var(--rule)}
.prose ul,.prose ol{padding-left:1.2rem;margin:.4rem 0}
.prose li{margin:.15rem 0}
.prose p{margin:.4rem 0}

/* ---- print ---- */
@page{ size:A4; margin:13mm 11mm; }
@media print{
  :root{
    --paper:#fff; --ink:#000; --soft:#333; --faint:#555;
    --rule:#bbb; --rule2:#888; --codebg:#fbfbfb; --accent:#0a4a55;
  }
  body{font-size:10pt}
  .wrap{max-width:none;padding:0;margin:0}
  .noprint{display:none !important}
  pre{font-size:8.2pt;border-color:#ccc;break-inside:auto}
  h2{break-after:avoid;page-break-after:avoid}
  .path{break-after:avoid;page-break-after:avoid}
  .file{break-inside:auto}
  .toc{break-after:page;page-break-after:always}
  a{color:inherit;text-decoration:none}
  .prose tr{break-inside:avoid}
}

.hint{
  background:var(--codebg);border:1px solid var(--rule);
  border-left:4px double var(--rule2);
  border-radius:3px;padding:.7rem .9rem;margin:0 0 1.4rem;
  font-size:.9em;color:var(--soft)
}
.hint b{color:var(--ink)}
"""


def md_to_html(md: str) -> str:
    """Small markdown renderer -- enough for the index files we wrote:
    headings, tables, lists, code spans, bold, italic, hr, paragraphs."""
    lines = md.split("\n")
    out, i = [], 0

    def inline(s):
        s = html.escape(s)
        s = re.sub(r"`([^`]+)`", r"<code>\1</code>", s)
        s = re.sub(r"\*\*([^*]+)\*\*", r"<strong>\1</strong>", s)
        s = re.sub(r"(?<!\*)\*([^*\n]+)\*(?!\*)", r"<em>\1</em>", s)
        s = re.sub(r"\[([^\]]+)\]\(([^)]+)\)", r"\1", s)   # links -> plain text
        return s

    while i < len(lines):
        ln = lines[i]
        if not ln.strip():
            i += 1
            continue
        if re.match(r"^-{3,}\s*$", ln):
            out.append("<hr>")
            i += 1
            continue
        m = re.match(r"^(#{1,6})\s+(.*)$", ln)
        if m:
            lvl = min(len(m.group(1)) + 2, 6)
            out.append(f"<h{lvl}>{inline(m.group(2))}</h{lvl}>")
            i += 1
            continue
        # table
        if ln.lstrip().startswith("|") and i + 1 < len(lines) and \
           re.match(r"^\s*\|[\s:|-]+\|\s*$", lines[i + 1]):
            def cells(row):
                r = row.strip()
                if r.startswith("|"):
                    r = r[1:]
                if r.endswith("|"):
                    r = r[:-1]
                return [c.strip() for c in re.split(r"(?<!\\)\|", r)]
            hdr = cells(ln)
            i += 2
            body = []
            while i < len(lines) and lines[i].lstrip().startswith("|"):
                body.append(cells(lines[i]))
                i += 1
            t = ["<table><thead><tr>"]
            t += [f"<th>{inline(c)}</th>" for c in hdr]
            t.append("</tr></thead><tbody>")
            for row in body:
                t.append("<tr>" + "".join(f"<td>{inline(c)}</td>" for c in row) + "</tr>")
            t.append("</tbody></table>")
            out.append("".join(t))
            continue
        # list
        if re.match(r"^\s*[-*]\s+", ln):
            items = []
            while i < len(lines) and re.match(r"^\s*[-*]\s+", lines[i]):
                items.append(inline(re.sub(r"^\s*[-*]\s+", "", lines[i])))
                i += 1
            out.append("<ul>" + "".join(f"<li>{x}</li>" for x in items) + "</ul>")
            continue
        if re.match(r"^\s*\d+\.\s+", ln):
            items = []
            while i < len(lines) and re.match(r"^\s*\d+\.\s+", lines[i]):
                items.append(inline(re.sub(r"^\s*\d+\.\s+", "", lines[i])))
                i += 1
            out.append("<ol>" + "".join(f"<li>{x}</li>" for x in items) + "</ol>")
            continue
        # paragraph
        para = []
        while i < len(lines) and lines[i].strip() and \
                not lines[i].lstrip().startswith("|") and \
                not re.match(r"^(#{1,6})\s|^\s*[-*]\s|^\s*\d+\.\s|^-{3,}\s*$", lines[i]):
            para.append(lines[i].strip())
            i += 1
        if para:
            out.append(f"<p>{inline(' '.join(para))}</p>")
        else:
            i += 1
    return "\n".join(out)


def emit_html(cat, files, idx, anch):
    slug, title, blurb, roots, _order = cat
    P = []
    P.append(f"<title>{html.escape(title)} — 3ERSAN EL GANNA</title>")
    P.append(f"<style>{CSS}</style>")
    P.append('<div class="wrap">')
    P.append(f"<h1>{idx}. {html.escape(title)}</h1>")
    P.append(f'<p class="blurb">{html.escape(blurb)}</p>')
    P.append(f'<p class="meta">{len(files)} files · 3ERSAN EL GANNA team reference</p>')
    P.append('<div class="hint noprint"><b>To make the PDF:</b> Ctrl+P → '
             'Destination <b>Save as PDF</b> → turn ON <b>Background graphics</b> '
             '(for the coloured code) and <b>Headers and footers</b> (for page '
             'numbers) → Margins <b>Default</b>. This box does not print.</div>')

    # TOC
    P.append('<div class="toc"><h2>Contents</h2>')
    last_group = "\x00"                 # sentinel: no real group can equal it
    open_ol = False
    for k, p in enumerate(files, 1):
        g = group_of(p, roots)
        if g != last_group:
            if open_ol:
                P.append("</ol>")
            if g:
                P.append(f'<div class="grp">{html.escape(g)}</div>')
            P.append("<ol>")
            open_ol = True
            last_group = g
        anchor = anch[p]
        P.append(f'<li><a href="#{anchor}">'
                 f'<span>{html.escape(nice_name(p))}</span>'
                 f'<span class="num">{idx}.{k}</span></a></li>')
    if open_ol:
        P.append("</ol>")
    P.append("</div>")

    # bodies
    for k, p in enumerate(files, 1):
        anchor = anch[p]
        P.append(f'<div class="file" id="{anchor}">')
        P.append(f"<h2>{idx}.{k} &nbsp;{html.escape(nice_name(p))}</h2>")
        P.append(f'<p class="path">{html.escape(p)}</p>')
        body = read(p)
        if p.endswith(".md"):
            P.append(f'<div class="prose">{md_to_html(body)}</div>')
        else:
            P.append(f"<pre><code>{highlight_cpp(body.rstrip())}</code></pre>")
        P.append("</div>")

    P.append("</div>")
    return "\n".join(P) + "\n"


# ---------------------------------------------------------------------------
def main():
    os.makedirs(OUT, exist_ok=True)
    built = []
    for i, cat in enumerate(CATEGORIES):
        slug, title, blurb, roots, _ = cat
        files = collect(cat)
        if not files:
            print(f"  !! no files for {slug}", file=sys.stderr)
            continue
        anch = anchors_for(files)
        stem = f"{i:02d}-{slug}"
        with open(os.path.join(OUT, stem + ".md"), "w",
                  encoding="utf-8", newline="\n") as f:
            f.write(emit_md(cat, files, i, anch))
        with open(os.path.join(OUT, stem + ".html"), "w",
                  encoding="utf-8", newline="\n") as f:
            f.write(emit_html(cat, files, i, anch))
        built.append((stem, title, len(files)))
        print(f"  {stem:<22} {len(files):>3} files")

    # cover page
    C = ["<title>3ERSAN EL GANNA — Team Reference</title>",
         f"<style>{CSS}</style>", '<div class="wrap">',
         "<h1>3ERSAN EL GANNA</h1>",
         '<p class="blurb">Competitive programming team reference. '
         'Nine documents; open one and print it.</p>',
         '<div class="hint"><b>To make a PDF:</b> open a document below, Ctrl+P, '
         'Destination <b>Save as PDF</b>. Turn ON <b>Background graphics</b> so the '
         'code colouring survives, and <b>Headers and footers</b> if you want page '
         'numbers.</div>',
         '<div class="toc"><ol>']
    total = 0
    for stem, title, n in built:
        total += n
        C.append(f'<li><a href="{stem}.html"><span>{html.escape(title)}</span>'
                 f'<span class="num">{n} files</span></a></li>')
    C.append(f"</ol></div><p class=\"meta\">{total} files total.</p></div>")
    with open(os.path.join(OUT, "index.html"), "w",
              encoding="utf-8", newline="\n") as f:
        f.write("\n".join(C) + "\n")

    R = ["# Printable reference", "",
         "Generated by `tools/build_reference.py`. Do not edit these by hand —",
         "edit the source files and re-run the script.", "",
         "Open `index.html` in a browser, pick a document, Ctrl+P, Save as PDF.",
         "Turn ON *Background graphics* so the code colouring survives.", "",
         "| # | document | files | markdown | print |", "|---|---|---|---|---|"]
    for i, (stem, title, n) in enumerate(built):
        R.append(f"| {i} | {title} | {n} | [`{stem}.md`]({stem}.md) "
                 f"| [`{stem}.html`]({stem}.html) |")
    R += ["", f"**{total} files total.**", "",
          "Files deliberately excluded (the second copy of an algorithm the repo",
          "keeps twice) are listed in `DROP` at the top of the build script.", ""]
    with open(os.path.join(OUT, "README.md"), "w",
              encoding="utf-8", newline="\n") as f:
        f.write("\n".join(R))

    print(f"\n  {total} files across {len(built)} documents -> reference/")


if __name__ == "__main__":
    main()

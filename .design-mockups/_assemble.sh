#!/bin/sh
name="$1"; body="$2"; props="$3"
caret=$(cat _caret.frag)
{
  printf '%s\n' '<!doctype html>' '<html>' '<head>' '  <meta charset="utf-8">' '  <script src="./support.js"></script>' '</head>' '<body>' '<x-dc>'
  cat _helmet.frag
  awk -v c="$caret" '{ gsub(/@CARET@/, c); print }' "$body"
  printf '%s\n' '</x-dc>'
  printf '%s\n' "<script data-dc-script data-props='$props'>"
  printf '%s\n' 'class Component extends DCLogic {' '  renderVals() {' "    return { accent: this.props.accent ?? '#ffd133' };" '  }' '}'
  printf '%s\n' '</script>' '</body>' '</html>'
} > "$name.dc.html"
echo "wrote $name.dc.html"

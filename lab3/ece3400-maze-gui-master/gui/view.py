import pandas as pd
import webbrowser, os
# Prevent cell truncating when creating the HTML table
pd.set_option('display.max_colwidth', 1)

class View():
  def __init__(self, rows, cols, open_browser=False, refresh_rate=1, default_sprite='./sprites/unexplored.jpg'):
    self._refresh_rate = refresh_rate
    self._sprite_layout = pd.DataFrame([self._add_tag(default_sprite) for c in xrange(cols)] for r in xrange(rows))
    self._render()
    if open_browser:
      webbrowser.open('file://' + os.path.realpath('./maze.html'))

  def _add_tag(self, sprite):
    return '<img src="%s"/>' % sprite

  def update_cell(self, row, col, sprite):
    self._sprite_layout.set_value(row, col, self._add_tag(sprite))
    self._render()

  def _render(self):
    # Render the sprite layout as an HTML document
    with open('maze.html', 'w') as maze_html:
      # The meta tag causes the browser to automatically refresh the page
      maze_html.write('<meta http-equiv="refresh" content="%.2f"/>\n' % self._refresh_rate)
      self._sprite_layout.to_html(maze_html, escape=False, header=False, index=False)

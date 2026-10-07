import pcbnew

PATH = "/home/cgu/Documents/FES_Board/FESboard/FESboard.kicad_pcb"

SIZE_MM = 0.5
THICK_MM = 0.08

board = pcbnew.LoadBoard(PATH)

sz = pcbnew.VECTOR2I(pcbnew.FromMM(SIZE_MM), pcbnew.FromMM(SIZE_MM))
th = pcbnew.FromMM(THICK_MM)

ref_changed = 0
val_hidden = 0

for fp in board.GetFootprints():
    ref = fp.Reference()
    ref.SetTextSize(sz)
    ref.SetTextThickness(th)
    ref.SetVisible(True)
    ref_changed += 1

    val = fp.Value()
    val.SetVisible(False)
    val_hidden += 1

pcbnew.SaveBoard(PATH, board)

print(f"references resized: {ref_changed}")
print(f"values hidden:      {val_hidden}")

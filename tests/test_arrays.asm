[1, 2, 3, 4, 5, 255, 254, 253, 252, 251, 3, 232, 7, 208, 11, 184, 15, 160, 19, 136]
Label(id='label3')
PushZeros(n=0)
PushConstantV(value=0)
PopLocal(sfa=0)
Label(id='label2')
PushLocal(sfa=0)
PushConstantV(value=5)
CompareLT()
JumpVIfNot(address=LabelRef('label1'))
PushLocal(sfa=0)
GlobalLoad16(nargs=1, popoffset=True, address=10)
PushLocal(sfa=0)
GlobalLoadS8(nargs=1, popoffset=True, address=5)
PushLocal(sfa=0)
GlobalLoadU8(nargs=1, popoffset=True, address=0)
PushConstantV(value=3)
CallUserFunction(funcid=1)
DropValue()
PushLocal(sfa=0)
PushConstantV(value=1)
Add()
PopLocal(sfa=0)
JumpV(address=LabelRef('label2'))
Label(id='label1')
PushConstantV(value=0)
CallUserFunction(funcid=2)
DropValue()
Return0()
PushZeros(n=0)
CallV(address=LabelRef('label3'))
DropValue()
PushConstantV(value=0)
PopLocal(sfa=0)
Label(id='label5')
PushLocal(sfa=0)
PushConstantV(value=5)
CompareLT()
JumpVIfNot(address=LabelRef('label4'))
PushLocal(sfa=0)
Bury(k=0)
GlobalLoadU8(nargs=1, popoffset=True, address=0)
PushConstantV(value=1)
Add()
Dig(k=0)
GlobalStoreU8(nargs=1, popoffset=True, address=0)
PushLocal(sfa=0)
Bury(k=0)
GlobalLoad16(nargs=1, popoffset=True, address=10)
PushLocal(sfa=0)
Bury(k=0)
GlobalLoadS8(nargs=1, popoffset=True, address=5)
PushConstantV(value=1)
Sub()
Bury(k=1)
Dig(k=0)
GlobalStoreS8(nargs=1, popoffset=True, address=5)
Sub()
Dig(k=0)
GlobalStore16(nargs=1, popoffset=True, address=10)
PushLocal(sfa=0)
PushConstantV(value=1)
Add()
PopLocal(sfa=0)
JumpV(address=LabelRef('label5'))
Label(id='label4')
CallV(address=LabelRef('label3'))
DropValue()
PushConstantV(value=0)
PopLocal(sfa=0)
Label(id='label10')
PushLocal(sfa=0)
PushConstantV(value=5)
CompareLT()
JumpVIfNot(address=LabelRef('label7'))
PushConstantV(value=200)
PushLocal(sfa=0)
Add()
PushLocal(sfa=0)
GlobalStoreU8(nargs=1, popoffset=True, address=0)
PushConstantV(value=100)
PushLocal(sfa=0)
PushConstantV(value=1)
BitwiseAnd()
JumpVIfNot(address=LabelRef('label8'))
PushConstantV(value=-1)
JumpV(address=LabelRef('label9'))
Label(id='label8')
PushConstantV(value=1)
Label(id='label9')
Mul()
PushLocal(sfa=0)
GlobalStoreS8(nargs=1, popoffset=True, address=5)
PushConstantV(value=111)
PushLocal(sfa=0)
PushConstantV(value=1)
Add()
Mul()
PushLocal(sfa=0)
GlobalStore16(nargs=1, popoffset=True, address=10)
PushLocal(sfa=0)
PushConstantV(value=1)
Add()
PopLocal(sfa=0)
JumpV(address=LabelRef('label10'))
Label(id='label7')
CallV(address=LabelRef('label3'))
DropValue()
Return0()
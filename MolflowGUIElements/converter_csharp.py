#To convert a visual Studio form file into SDL framework gui
import sys #file IO
import pdb

SDLtypes = {"ComboBox":"GLCombo","GroupBox":"GLTitledPanel","Button":"GLButton","Label":"GLLabel","TextBox":"GLTextField","CheckBox":"GLToggle"}
source=open(sys.argv[1],'r') #Form1.h or similar
lines=source.readlines()
source.close()
formName="Form1"
formTitle="Form1 Untitled"

class Control(object):
	"""A typical control"""
	def __init__(self,name,type):
		self.name=name
		self.type=type
		self.SDLtype=SDLtypes[self.type]
		self.parentId=-1
		self.posX=0
		self.posY=0
		self.sizeX=0
		self.sizeY=0
		self.text=""
		self.added=0
		self.isParent=0
		self.items=[]
	def __str__(self):
		returnStr = self.name+" = new "+self.SDLtype+"("
		if self.SDLtype=="GLButton" or self.SDLtype=="GLToggle" or self.SDLtype=="GLTextField":
			returnStr+="0,"
		if self.SDLtype=="GLCombo":
			returnStr+="0);\n"
		else:
			returnStr+="\""+self.text+"\");\n"
		if self.parentId==-1:
			returnStr+= self.name+"->SetBounds("
		else:
			returnStr+= controls[self.parentId].name+"->SetCompBounds("+self.name+","
		returnStr+=str(self.posX)+","+str(self.posY)+","+str(self.sizeX)+","+str(self.sizeY)+");\n"
		if self.parentId>=0:
			returnStr+=controls[self.parentId].name+"->"
		returnStr+="Add("+self.name+");\n"
		if self.type=="ComboBox":
			returnStr+=self.name+"->SetSize("+str(len(self.items))+");\n"
			index=0
			for item in self.items:
				returnStr+=self.name+"->SetValueAt("+str(index)+',"'+item+'");\n'
				index+=1
		return returnStr
		
controls=[]
map={}

l=len(lines)

#Search "end region" for declarations
foundEndRegion = False
for index, line in enumerate(lines):
	if line.strip()=="#endregion":
		foundEndRegion = True
	if (not foundEndRegion):
		continue
	if line.strip().startswith("private System.Windows.Forms."): #new control
		#pdb.set_trace()
		parts=line.split(".")
		type=parts[-1].split(" ")[0]
		name=parts[-1].split(" ")[1].split(";")[0]
		
		try:
			SDLtypes[type]
			newControl=Control(name,type)
			map[name]=len(controls)
			controls.append(newControl)
		except KeyError:
			print("Skipped object "+name+" of type "+type+"\n")
		

#Parameter controls now that they are added
for index, line in enumerate(lines):	
	if line.strip().startswith("this."):
		elementName=line.split(".")[1].split(" ")[0]
		if elementName=="Name":
			formName=line.split("\"")[1]
		elif elementName=="Text":
			formTitle=line.split("\"")[1]
		elif elementName=="Controls" and line.split(".")[2]=="Add(this":
			#pdb.set_trace()
			controls[map[line.split(".")[3].split(");")[0]]].added=1
		elif elementName=="ClientSize":
						formWidth=int(float(line.split(",")[0].split("(")[1]))
						formHeight=int(float(line.split(",")[1].split(");")[0]))
		else:
			try:
				elementId=map[elementName]
				#print line
				if len(line.split("."))>2:
					propertyName=line.split(".")[2]
					if propertyName.startswith("Location"):
						controls[elementId].posX=int(float(line.split(",")[0].split("(")[1]))
						controls[elementId].posY=int(float(line.split(",")[1].split(");")[0]))
					elif propertyName.startswith("Size"):
						controls[elementId].sizeX=int(float(line.split(",")[0].split("(")[1]))
						controls[elementId].sizeY=int(float(line.split(",")[1].split(");")[0]))
					elif propertyName.startswith("Text ="):
						controls[elementId].text=propertyName.split("\"")[1].split("\";")[0]
					elif propertyName==("Controls") and len(line.split("."))>3 and line.split(".")[3].startswith("Add(this"):
						#pdb.set_trace()
						childName=line.split(".Controls.Add(this.")[1].split(");")[0]
						controls[map[childName]].parentId=elementId
						controls[map[childName]].added=1
						controls[elementId].isParent=1
					elif propertyName==("Items") and len(line.split("."))>3 and line.split(".")[3].startswith("AddRange(new object[] {"):
						#pdb.set_trace()
						startIndex = index
						fullLine = line.strip()
						while not (lines[startIndex].strip().endswith("});")):
							#pdb.set_trace()
							startIndex+=1
							fullLine += " " + lines[startIndex].strip()
						#pdb.set_trace()
						controls[elementId].items=fullLine.split('[] { "')[1].split('"});')[0].split('", "')
			except KeyError:
				dummy=1
		
#pdb.set_trace()
f_out=open(sys.argv[1]+"_out.txt",'w') #output file
f_out.write(formName+".h:\n\n")

#pdb.set_trace()
for control in controls:
	if control.added:
		f_out.write(control.SDLtype+"\t*"+control.name+";\n")
f_out.write("\n\n\n"+formName+".cpp:\n\n")
f_out.write("int wD = "+str(formWidth+15)+";\n")
f_out.write("int hD = "+str(formHeight+15)+";\n")

#first write parents
for control in controls:
	if control.added and control.isParent:
		f_out.write(str(control))

#then everything else
for control in controls:
	if control.added and not control.isParent:
		f_out.write(str(control)+'\n')
f_out.write("SetTitle(\""+formTitle+"\");\n")
f_out.write("// Center dialog\n")
f_out.write("int wS,hS;\n")
f_out.write("GLToolkit::GetScreenSize(&wS,&hS);\n")
f_out.write("int xD = (wS-wD)/2;\n")
f_out.write("int yD = (hS-hD)/2;\n")
f_out.write("SetBounds(xD,yD,wD,hD);\n")
f_out.close()

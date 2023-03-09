class Person:
    def __init__(self,name,addr,workaddr,work,phone,email):
        self.name=name
        self.addr=addr
        self.workaddr=workaddr
        self.work=work
        self.phone=phone
        self.email=email
        
    def display(self):
        print("{1} {2} {3} {4} {5}".format(self.name,self.addr,self.workaddr,self.work,self.phone,self.email))
        
    def name(self):
        return self.name
        
        
class PersonManager:
    def __init__(self):
        self.persons=[]
    def add_person(self,person):
        self.persons.append(person)
    
    def del_person(self,name):
        for i in range(len(self.persons)):
            if self.persons[i].name()==name:
                self.persons.remove(i)
                
        
p =Person('123','123','123','123','123','123')
p.display()
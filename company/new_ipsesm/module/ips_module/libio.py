class file_io_controller:
  def __init__(self,save_path):
    self.save_path = save_path

  def save_file(self, file_name, data):
    fp = open(file_name,"w")
    fp.write(data)
    fp.close()


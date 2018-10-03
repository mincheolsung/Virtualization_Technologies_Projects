import boto3
import botocore
import sysv_ipc
import sys

class MyS3:
	# CLASS ATTRIBUTE
	# self.s3
	# self.bucket
	# self.name
	# self.client

	def __init__(self, s3, client):
		self.s3 = s3 #boto3.resource('s3')
		self.client = client #boto3.client('s3')
		for bucket in s3.buckets.all():
			self.bucket = bucket
			self.name = bucket.name

	def put(self, key, value):
		try:
			self.bucket.put_object(Key=key, Body=value)
			return "0"
		except botocore.exceptions.ClientError as e:
			if e.response['Error']['Code'] == "404":
				return "1"
			else:
				return "1"

	def get(self, key, memory_value, memory_len):
		try:
			data = self.client.get_object(Bucket=self.name, Key=key)
			length = data['ResponseMetadata']['HTTPHeaders']['content-length']
			if (int(length) > 4096):
				return "2"
			memory_len.write(length)
			value = data['Body'].read()
			memory_value.write(value)
			return "0"
		except botocore.exceptions.ClientError as e:
			if e.response['Error']['Code'] == "NoSuchKey":
				return "1"
			else:
				return "3"

	def delete(self, key):
		try:
			self.client.get_object(Bucket=self.name, Key=key)
			self.client.delete_object(Bucket=self.name, Key=key)
			return "0"
		except botocore.exceptions.ClientError as e:
			if e.response['Error']['Code'] == "NoSuchKey":
				return "1"
			else:
				return "1"

def main():
	# Let's use Amazon S3
	myS3 = MyS3(boto3.resource('s3'), boto3.client('s3',use_ssl=False))

	# Create shared memory object
	memory_key = sysv_ipc.SharedMemory(123456)
	memory_return = sysv_ipc.SharedMemory(123458)
	
	# Read value from shared memory
	key = memory_key.read()
	
	if sys.argv[1] == 'delete':
		ret = myS3.delete(key)

	elif sys.argv[1] == 'put':
		memory_value = sysv_ipc.SharedMemory(123457)
		value = memory_value.read()
		ret = myS3.put(key, value)

	elif sys.argv[1] == 'get':
		memory_value = sysv_ipc.SharedMemory(123457)
		memory_len = sysv_ipc.SharedMemory(123459)
		ret = myS3.get(key, memory_value, memory_len)
	
	else:
		print('else')

	memory_return.write(ret)

if __name__ == "__main__":
	main()

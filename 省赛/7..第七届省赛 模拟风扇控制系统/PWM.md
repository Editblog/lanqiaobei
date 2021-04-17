[![](http://47.95.246.149/wp-content/uploads/2021/04/wp_editor_md_96254f8db84f68321a1577a7284b0338.jpg)](http://47.95.246.149/wp-content/uploads/2021/04/wp_editor_md_96254f8db84f68321a1577a7284b0338.jpg)


[![](http://47.95.246.149/wp-content/uploads/2021/04/wp_editor_md_4704ac88ebc5df74c778ac242e491841.jpg)](http://47.95.246.149/wp-content/uploads/2021/04/wp_editor_md_4704ac88ebc5df74c778ac242e491841.jpg)


方法1：直接置位

```c
	if(count==num)
	{
		Y4;P0 = 0xff;
	}
	else if(count==100)
	{
		Y4;P0 = 0xfe;
		count = 0;
	}
```

方法2：

```c
	if(count<=pwm)
	{
		Y4;P0 = 0xfe;
	}
	else if(count<=100)
	{
		Y4;P0 = 0xff;
	}
	else if(count==100)
	{
		Y4;P0 = 0xfe;
		count=0;
	}
```
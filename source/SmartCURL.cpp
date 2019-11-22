#include "PlatformPrecomp.h"
#include "SmartCURL.h"
#include "util/MiscUtils.h"
#include "util//TextScanner.h"

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *pThisInstance)
{
	size_t realsize = size * nmemb;
	SmartCURL *pCURLInstance = (SmartCURL *)pThisInstance;

	char *ptr = (char *)realloc(pCURLInstance->m_pReceiveBuff, pCURLInstance->m_receivedSize + realsize + 1);
	if (ptr == NULL) {
		/* out of memory! */
		printf("not enough memory (realloc returned NULL)\n");
		return 0;
	}

	pCURLInstance->m_pReceiveBuff = ptr;
	memcpy(&(pCURLInstance->m_pReceiveBuff[pCURLInstance->m_receivedSize]), contents, realsize);
	pCURLInstance->m_receivedSize += realsize;
	pCURLInstance->m_pReceiveBuff[pCURLInstance->m_receivedSize] = 0;
	return realsize;
}

SmartCURL::SmartCURL()
{
}

SmartCURL::~SmartCURL()
{
	SAFE_FREE(m_pReceiveBuff);
}

void SmartCURL::Update()
{
	if (m_CURL_handles_still_running == 0) return;
	CURLMcode mc = curl_multi_perform(m_CURL_multi_handle, &m_CURL_handles_still_running);

	if (mc != CURLM_OK && mc != CURLM_CALL_MULTI_PERFORM)
	{
		LogMsg("CURL error");
	}

	CURLMsg *msg; /* for picking up messages with the transfer status */
	int msgs_left; /* how many messages are left */

	/* See how the transfers went */
	while ((msg = curl_multi_info_read(m_CURL_multi_handle, &msgs_left))) 
	{
		if (msg->msg == CURLMSG_DONE) 
		{
			int http_status_code = 0;
			curl_easy_getinfo(msg->easy_handle, CURLINFO_RESPONSE_CODE, &http_status_code);
			SmartCURL *pMe;
			curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE, &pMe);
			assert(pMe == this &&"This should be the case");

			//LogMsg("CURL done, HTTP status: %d, downloaded %d bytes. ", http_status_code, pMe->m_receivedSize);
			//LogMsg("Data is %s", pMe->m_pReceiveBuff);
			curl_multi_remove_handle(m_CURL_multi_handle, msg->easy_handle);
			curl_easy_cleanup(m_CURL_handle);
		}
	}

	if (m_CURL_handles_still_running == 0)
	{
		//finish up
		curl_multi_cleanup(m_CURL_multi_handle);
		//curl_global_cleanup();
		//LogMsg("CURL cleaned up");
	}
}
void AddText(const char *tex, const char *filename);

void dump(const char *text,
	FILE *stream, unsigned char *ptr, size_t size)
{
	size_t i;
	size_t c;
	unsigned int width = 0x10;

	LogMsgNoCR( "%s, %10.10ld bytes (0x%8.8lx): %s\n",
		text, (long)size, (long)size, ptr);

	return;
	for (i = 0; i < size; i += width) {
		LogMsgNoCR("%4.4lx: ", (long)i);

		/* show hex to the left */
		for (c = 0; c < width; c++) {
			if (i + c < size)
				LogMsgNoCR("%02x ", ptr[i + c]);
			else
				LogMsgNoCR("   ");
		}

		/* show data on the right */
		for (c = 0; (c < width) && (i + c < size); c++) 
		{
			char x = (ptr[i + c] >= 0x20 && ptr[i + c] < 0x80) ? ptr[i + c] : '.';
			LogMsgNoCR("%c", c);
		}
		LogMsgNoCR("\n");
	}
}

static
int my_trace(CURL *handle, curl_infotype type,
	char *data, size_t size,
	void *userp)
{
	const char *text;
	(void)handle; /* prevent compiler warning */
	(void)userp;

	switch (type) {
	case CURLINFO_TEXT:
		LogMsgNoCR("== Info: %s", data);
	default: /* in case a new one is introduced to shock us */
		return 0;

 	case CURLINFO_HEADER_OUT:
 		text = "=> Send header";
 		break;
// 	case CURLINFO_DATA_OUT:
// 		text = "=> Send data";
// 		break;
// 	case CURLINFO_SSL_DATA_OUT:
// 		text = "=> Send SSL data";
// 		break;
	case CURLINFO_HEADER_IN:
		text = "<= Recv header";
 		break;
// 	case CURLINFO_DATA_IN:
// 		text = "<= Recv data";
// 		break;
// 	case CURLINFO_SSL_DATA_IN:
// 		text = "<= Recv SSL data";
// 		break;
	}

	dump(text, stderr, (unsigned char *)data, size);
	return 0;
}

int SmartCURL::Start()
{
	if (m_CURL_handles_still_running != 0)
	{
		LogMsg("Warning: CURL activity already in progress");
	}

	static bool oneTimeInittedDone = false;

	if (!oneTimeInittedDone)
	{
		curl_global_init(CURL_GLOBAL_ALL);
		oneTimeInittedDone = true;
	}

	m_pReceiveBuff = (char*)malloc(1); 
	m_receivedSize = 0;    

	m_CURL_handle = curl_easy_init();
	m_CURL_multi_handle = curl_multi_init();
	//curl -v -s -H "Content-Type: application/json" "https://vision.googleapis.com/v1/images:annotate?key=googlekeygoeshere" -d@request.json

	string url = "https://vision.googleapis.com/v1/images:annotate?key=googlekeygoeshere";
	//url = "https://rtsoft.com/ip.php";
	 
	 postData = R"({
  "requests":[
    {
      "image":{
        "source":{
          "imageUri":
            "https://www.google.com/images/branding/googlelogo/2x/googlelogo_color_272x92dp.png"
        }
      },
      "features":[
        {
          "type":"LOGO_DETECTION",
          "maxResults":1
        }
      ]
    }
  ]
}
)";

	curl_easy_setopt(m_CURL_handle, CURLOPT_VERBOSE, 1L);
	curl_easy_setopt(m_CURL_handle, CURLOPT_DEBUGFUNCTION, my_trace);
	curl_easy_setopt(m_CURL_handle, CURLOPT_URL, url.c_str());
	//curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0);
	curl_easy_setopt(m_CURL_handle, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(m_CURL_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
	curl_easy_setopt(m_CURL_handle, CURLOPT_WRITEDATA, this);
	curl_easy_setopt(m_CURL_handle, CURLOPT_USERAGENT, "gametrans-agent/1.0");
	curl_easy_setopt(m_CURL_handle, CURLOPT_PRIVATE, this);
	curl_easy_setopt(m_CURL_handle, CURLOPT_POST, 1L);
	curl_easy_setopt(m_CURL_handle, CURLOPT_POSTFIELDS, postData.c_str());
	//manually set the certs otherwise it can't find it (a windows only issue?)
	curl_easy_setopt(m_CURL_handle, CURLOPT_CAINFO, "curl-ca-bundle.crt");
	
	//curl_easy_setopt(curl_handle, CURLOPT_READFUNCTION, ReadMemoryCallback);
	//curl_easy_setopt(curl_handle, CURLOPT_READDATA, this);
	
	struct curl_slist *chunk = NULL;

	/* Add a custom header */
	chunk = curl_slist_append(chunk, "Accept: */*");
	chunk = curl_slist_append(chunk, "Content-Type: application/json");

	/* set our custom set of headers */
	curl_easy_setopt(m_CURL_handle, CURLOPT_HTTPHEADER, chunk);

#ifdef USE_CHUNKED
	{
		struct curl_slist *chunk = NULL;

		chunk = curl_slist_append(chunk, "Transfer-Encoding: chunked");
		res = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
		/* use curl_slist_free_all() after the *perform() call to free this
		   list again */
	}
#else
	/* Set the expected POST size. If you want to POST large amounts of data,
	   consider CURLOPT_POSTFIELDSIZE_LARGE */
	//curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDSIZE, (long)wt.sizeleft);
#endif

#ifdef DISABLE_EXPECT
	/*
	  Using POST with HTTP 1.1 implies the use of a "Expect: 100-continue"
	  header.  You can disable this header with CURLOPT_HTTPHEADER as usual.
	  NOTE: if you want chunked transfer too, you need to combine these two
	  since you can only set one list of headers with CURLOPT_HTTPHEADER. */

	  /* A less good option would be to enforce HTTP 1.0, but that might also
		 have other implications. */
	{
		struct curl_slist *chunk = NULL;

		chunk = curl_slist_append(chunk, "Expect:");
		res = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
		/* use curl_slist_free_all() after the *perform() call to free this
		   list again */
	}
#endif

	CURLMcode result  = curl_multi_add_handle(m_CURL_multi_handle, m_CURL_handle);
	if (result != CURLM_OK)
	{
		LogMsg("SmartCURL error: %s", curl_multi_strerror(result));
	}
	
	result = curl_multi_perform(m_CURL_multi_handle, &m_CURL_handles_still_running);

	if (result != CURLM_OK)
	{
		LogMsg("SmartCURL error: %s", curl_multi_strerror(result));
	}
	
	if (m_CURL_handles_still_running == 0)
	{
		LogMsg("SmartCURL - Error, seemed like nothing happened.");
	}
	return 0;
}
